#include "Mod.h"
#include "PlayerControl.h"
#include "APProtocol.h"
#include "ItemEffects.h"
#include "CStreaming.h"
#include "CPools.h"
#include <CRadar.h>

Mod::Mod()
{
	m_apSocket.connectToServer("127.0.0.1", 12345);

	m_persistentSubsystems = { &m_checkListener, &m_checkGiver, &m_blipManager, &m_receivedItemLog, &m_trapHandler };
}

void Mod::start()
{
    // The order of these phases is load-bearing; each one's comment says why.
    m_apSocket.update();
    pollDeathLink();

    bool worldWiped = updateWorldState();
    persistAndRestoreState(worldWiped);

    // Detection has to run before the respawn top-up: it re-asserts the trackers' max-health
    // override for this tick, which the top-up then heals to.
    CheckEvent event = m_checkListener.update();
    applyRespawnHealthTopUp();

    sendChecksToAP(event);
    updateGameplaySystems();
    updateMissionBlockers();

    parseIncomingMessages();
}

void Mod::pollDeathLink()
{
    if (m_deathLinkHandler.update())
    {
        m_apSocket.sendToServer(APProtocol::playerDied());
    }
}

// Returns true when the world was rebuilt this tick (a load or a new game).
bool Mod::updateWorldState()
{
    // Both signals are polled every tick (no short-circuit): the object sentinel catches loads,
    // while the blip pool still catches a New Game that clears the radar.
    // Wipe detection must run BEFORE the blip manager touches anything: after a load its handles
    // are stale, and acting on them would clear blips that now belong to the game.
    bool objectWiped = detectWorldWipe();
    if (objectWiped)
    {
        m_blipManager.onWorldWiped();
    }
    bool blipWiped = m_blipManager.render(collectBlipTargets());
    return blipWiped || objectWiped;
}

std::vector<BlipTarget> Mod::collectBlipTargets()
{
    std::vector<BlipTarget> targets;
    for (CollectibleTracker* collectible : m_checkListener.getCollectibles())
    {
        collectible->appendBlipTargets(targets);
    }
    if (CPlayerPed* player = FindPlayerPed())
    {
        rankByDistance(targets, player->GetPosition());
    }
    return targets;
}

// The hospital/police respawn refill recomputes max health from the game's internal stat,
// ignoring the Paramedic tracker's override - so on the respawn edge, top current health up to
// our max. Heals to whatever the current max is, so it's a no-op without the upgrade and also
// corrects the reverse case (respawn granting MORE than an unearned max).
void Mod::applyRespawnHealthTopUp()
{
    if (!m_deathLinkHandler.consumeRespawn()) return;

    if (CPlayerPed* player = FindPlayerPed())
    {
        player->m_fHealth = static_cast<float>(CWorld::Players[0].m_nMaxHealth);
    }
}

void Mod::updateGameplaySystems()
{
    m_ammuNationShop.update();
    m_trapHandler.update();
    m_checkGiver.update();

    if (m_autoSaveManager.update())
    {
        m_notificationOverlay.showAboveRadar("Archipelago: Autosaved (slot 8)");
    }

    int purchasedSlot = m_ammuNationShop.pollPurchasedSlot();
    if (purchasedSlot >= 0)
    {
        m_pendingShopChecks.push(purchasedSlot);
        m_notificationOverlay.show("Archipelago: Checked Ammu-Nation (" + std::string(shopItems[purchasedSlot].displayName) + ")");
    }
}

void Mod::updateMissionBlockers()
{
    // Self-heal: drop pointers whose objects have been destroyed (a load we failed to detect).
    if (m_blockersSpawned)
    {
        size_t liveCount = 0;
        for (CObject* blocker : m_missionBlockers)
        {
            if (CPools::ms_pObjectPool->IsObjectValid(blocker))
            {
                m_missionBlockers[liveCount++] = blocker;
            }
        }
        m_missionBlockers.resize(liveCount);
        m_blockersSpawned = !m_missionBlockers.empty();
    }

    // On a timer, not the world-wipe signal - that never fires for objects.
    if (++m_blockerScanTicks >= BLOCKER_SCAN_INTERVAL && FindPlayerPed())
    {
        m_blockerScanTicks = 0;
        adoptExistingBlockers();
    }

    if (m_checkGiver.getProgressiveMissionCounter() == 0 && !m_blockersSpawned)
    {
        spawnMissionBlockers();
        m_notificationOverlay.show("Note: You are out of Progressive Missions. Missions will be blocked until you unlock more.");
    }
    else if (m_checkGiver.getProgressiveMissionCounter() > 0 && m_blockersSpawned)
    {
        removeMissionBlockers();
    }
}

static std::string collectibleLabel(const std::string& t_type)
{
    if (t_type == "TAG") return "LS Tag";
    if (t_type == "SNAPSHOT") return "SF Snapshot";
    if (t_type == "HORSESHOE") return "LV Horseshoe";
    if (t_type == "OYSTER") return "Oyster";
    return t_type;
}

bool Mod::detectWorldWipe()
{
	bool wiped = false;

	if (m_worldSentinel)
	{
		// The pool slot can be recycled by an unrelated object after ours is destroyed, so
		// confirm the model too - otherwise a reused slot would look like our sentinel.
		bool stillOurs = CPools::ms_pObjectPool->IsObjectValid(m_worldSentinel)
			&& m_worldSentinel->m_nModelIndex == BLOCKER_MODEL_ID;
		if (!stillOurs)
		{
			wiped = true;
			m_worldSentinel = nullptr;
		}
	}

	if (!m_worldSentinel)
	{
		CPlayerPed* player = FindPlayerPed();
		if (!player) return wiped;

		CStreaming::RequestModel(BLOCKER_MODEL_ID, 0);
		CStreaming::LoadAllRequestedModels(false);

		m_worldSentinel = CObject::Create(BLOCKER_MODEL_ID);
		if (m_worldSentinel)
		{
			// Parked far below the player so it can never be seen or collided with.
			CVector pos = player->GetPosition();
			m_worldSentinel->SetPosition(CVector(pos.x, pos.y, pos.z - 500.0f));
			m_worldSentinel->SetIsStatic(true);
			m_worldSentinel->bStreamingDontDelete = true;
			m_worldSentinel->bIsVisible = false;
			m_worldSentinel->bUsesCollision = false;
			m_worldSentinel->m_nObjectType = OBJECT_MISSION;
			CWorld::Add(m_worldSentinel);
		}
	}

	return wiped;
}

void Mod::spawnMissionBlockers()
{
    CStreaming::RequestModel(BLOCKER_MODEL_ID, 0);
    CStreaming::RequestModel(BARRICADE_MODEL_ID, 0);
    CStreaming::LoadAllRequestedModels(false);

    for (const Position& pos : missionStartPos) {
        CObject* blocker = CObject::Create(BLOCKER_MODEL_ID);

        if (blocker) {
            blocker->SetPosition(CVector(pos.x, pos.y, pos.z));
            blocker->SetIsStatic(true);
            blocker->bStreamingDontDelete = true;
            blocker->bDistanceFade = true;
            blocker->bIsVisible = false;
            blocker->m_nObjectType = OBJECT_MISSION;
            CWorld::Add(blocker);
            m_missionBlockers.push_back(blocker);
        }

        CObject* barricade = CObject::Create(BARRICADE_MODEL_ID);

        if (barricade) {
            barricade->SetPosition(CVector(pos.x, pos.y, pos.z + BARRICADE_Z_OFFSET));
            barricade->SetIsStatic(true);
            barricade->bStreamingDontDelete = true;
            barricade->bDistanceFade = true;
            barricade->m_nObjectType = OBJECT_MISSION;
            CWorld::Add(barricade);
            m_missionBlockers.push_back(barricade);
        }
    }
    // Only latch when objects actually appeared. Right after a game load the models may not be
    // streamed in yet and CObject::Create can return null for every position - latching then
    // would leave the player with no blockers and no retry, so try again next tick instead.
    m_blockersSpawned = !m_missionBlockers.empty();
}

bool Mod::ownsBlocker(const CObject* t_object) const
{
    for (const CObject* blocker : m_missionBlockers)
    {
        if (blocker == t_object) return true;
    }
    return false;
}

bool Mod::isBlockerPosition(const CVector& t_position, int t_modelId) const
{
    for (const Position& spawn : missionStartPos)
    {
        float expectedZ = t_modelId == BARRICADE_MODEL_ID ? spawn.z + BARRICADE_Z_OFFSET : spawn.z;
        float dx = t_position.x - spawn.x;
        float dy = t_position.y - spawn.y;
        float dz = t_position.z - expectedZ;
        if (dx * dx + dy * dy + dz * dz < BLOCKER_POSITION_TOLERANCE_SQ) return true;
    }
    return false;
}

void Mod::adoptExistingBlockers()
{
    auto* pool = CPools::ms_pObjectPool;
    if (!pool) return;

    for (int i = 0; i < pool->m_nSize; ++i)
    {
        CObject* object = pool->GetAt(i);
        // The sentinel shares BLOCKER_MODEL_ID - adopting it would delete our wipe detector.
        if (!object || object == m_worldSentinel) continue;

        int modelId = object->m_nModelIndex;
        if (modelId != BLOCKER_MODEL_ID && modelId != BARRICADE_MODEL_ID) continue;
        if (!isBlockerPosition(object->GetPosition(), modelId)) continue;

        if (!ownsBlocker(object))
        {
            m_missionBlockers.push_back(object);
        }
    }

    m_blockersSpawned = !m_missionBlockers.empty();
}

void Mod::removeMissionBlockers()
{
    for (CObject* blocker : m_missionBlockers) {
        // Backstop against pointers that dangle because a game load destroyed the objects
        // without the load being detected. Note this can't catch a freed slot the new game
        // state has already reused - the restore-time reset above is the primary protection.
        if (!CPools::ms_pObjectPool->IsObjectValid(blocker)) continue;

        CWorld::Remove(blocker);
        delete blocker;
    }
    m_missionBlockers.clear();

    m_blockersSpawned = false;
}

void Mod::sendChecksToAP(CheckEvent t_event)
{
    switch (t_event)
    {
    case CheckEvent::Mission:
    {
        std::string missionIDStr = m_checkListener.getMissionID();
        if (m_apSocket.sendToServer(APProtocol::missionCheck(missionIDStr)))
        {
            if (m_checkListener.isStoryMission(parseIntOr(missionIDStr, -1)))
            {
                m_checkGiver.removeProgressiveMission();
            }
            m_checkListener.confirmMissionSent();
            // Arm an autosave now that the check is away and the counter has been spent, so
            // the save reflects the completed mission. It fires once the game is safe to save.
            m_autoSaveManager.requestSave();
        }
        break;
    }
    case CheckEvent::PickUp:
        if (m_apSocket.sendToServer(APProtocol::pickUpCheck()))
        {
            m_checkListener.confirmPickUpSent();
            m_notificationOverlay.show("Picked up an item");
        }
        break;
    case CheckEvent::Submission:
        if (m_apSocket.sendToServer(APProtocol::missionCheck(m_checkListener.getPendingSubmissionId())))
        {
            m_checkListener.confirmSubmissionSent();
            m_autoSaveManager.requestSave();
        }
        break;
    case CheckEvent::None:
        break;
    }

    // Collectibles (tags, snapshots) each know their own wire format, so one loop drains every
    // kind - adding another needs no change here.
    for (CollectibleTracker* collectible : m_checkListener.getCollectibles())
    {
        if (collectible->hasPending()
            && m_apSocket.sendToServer(collectible->buildCheckMessage(collectible->getPendingIndex())))
        {
            collectible->confirmSent();
        }
    }

    // Per-level submission checks (Paramedic/Firefighter/Vigilante levels 1-12) live outside
    // CheckListener's event system - send independently.
    if (m_checkListener.hasPendingSubmissionLevel())
    {
        if (m_apSocket.sendToServer(APProtocol::submissionLevelCheck(m_checkListener.getPendingSubmissionLevelSlot())))
        {
            m_checkListener.confirmSubmissionLevelSent();
        }
    }

    // Shop purchases live outside CheckListener's event system - send independently.
    if (m_pendingShopChecks.hasPending())
    {
        if (m_apSocket.sendToServer(APProtocol::shopCheck(m_pendingShopChecks.front())))
        {
            m_pendingShopChecks.confirm();
        }
    }
}

void Mod::parseIncomingMessages()
{
    std::string rawLine;
    while (m_apSocket.tryGetMessage(rawLine))
    {
        APProtocol::Message message = APProtocol::parse(rawLine);

        switch (message.kind)
        {
        case APProtocol::MessageKind::Status:
            m_notificationOverlay.show(message.text);
            break;

        // An item we found that belongs to another player's world.
        case APProtocol::MessageKind::ItemSent:
            m_notificationOverlay.show(message.text, NotificationIcon::ItemSent);
            break;

        case APProtocol::MessageKind::Locate:
            m_checkListener.locateCollectible(message.effect, message.index);
            if (message.index >= 0)
            {
                m_notificationOverlay.show("Locating " + collectibleLabel(message.effect)
                    + " #" + std::to_string(message.index + 1));
            }
            break;

        case APProtocol::MessageKind::ShopItem:
            m_ammuNationShop.setSlotContents(message.index, message.text);
            break;

        // Buffered rather than applied here: the log decides what this save is actually owed,
        // and batching the whole tick's delivery lets a re-grant be summarised in one line.
        case APProtocol::MessageKind::Give:
            m_receivedItemLog.recordDelivered(message.index, message.effect, message.text);
            break;

        case APProtocol::MessageKind::Control:
            applyControlMessage(message.effect, message.text);
            break;

        case APProtocol::MessageKind::Unknown:
            break;
        }
    }

    applyPendingItems();
}

void Mod::applyControlMessage(const std::string& t_name, const std::string& t_value)
{
    // Never deduplicated and never announced - these are events, not items the player owns.
    if (t_name == "death_link")
    {
        m_deathLinkHandler.setEnabled(t_value == "1");
    }
    else if (t_name == "deathlink_kill")
    {
        m_deathLinkHandler.killPlayer();
    }
    else if (t_name == "collectibles")
    {
        m_checkListener.setIncludedCollectibles(t_value);
    }
}

void Mod::applyPendingItems()
{
    // Nothing is applied until the save's mark has had its chance to load, which happens on the
    // first in-game tick. Applying while still in the menus would grant against a default mark
    // and then immediately roll back once the real save arrived.
    if (!m_firstInGameTickHandled) return;

    std::vector<ReceivedItem> pending = m_receivedItemLog.takePendingItems();
    if (pending.empty()) return;

    int restoredCount = 0;
    for (const ReceivedItem& item : pending)
    {
        if (applyItemEffect(item.effect, item.value, item.isNew) && !item.isNew)
        {
            restoredCount++;
        }
    }

    // A rollback can owe a save dozens of items at once; one line beats burying the screen.
    if (restoredCount > 0)
    {
        m_notificationOverlay.show("Archipelago: Restored " + std::to_string(restoredCount) + " items");
    }
}

bool Mod::applyItemEffect(const std::string& t_effectName, const std::string& t_value, bool t_isNew)
{
    const ItemEffectSpec* spec = findItemEffect(t_effectName);
    if (!spec) return false;

    // Traps are one-shot events, not possessions. Re-granting one to a rolled-back save would
    // punish the player a second time for an item they already suffered through.
    if (spec->effect == ItemEffect::Trap && !t_isNew) return true;

    switch (spec->effect)
    {
    case ItemEffect::Money:              m_checkGiver.giveMoney(parseIntOr(t_value, 0)); break;
    case ItemEffect::Weapon:             m_checkGiver.giveWeapon(t_value); break;
    case ItemEffect::ProgressiveMission: m_checkGiver.giveProgressiveMission(); break;
    case ItemEffect::ProgressiveMap:     m_checkGiver.giveProgressiveMap(); break;
    case ItemEffect::SubmissionCheck:    m_checkListener.submissionCheckWasReceived(spec->submissionId); break;
    case ItemEffect::WeaponMastery:      m_checkGiver.giveWeaponMastery(t_value); break;
    case ItemEffect::ArmorRefill:        m_checkGiver.giveArmorRefill(); break;
    case ItemEffect::CarRepair:          m_checkGiver.giveCarRepair(); break;
    case ItemEffect::Trap:               m_trapHandler.giveTrap(spec->trapName); break;
    }

    // Re-grants are counted and summarised by the caller instead.
    if (t_isNew)
    {
        std::string message = formatItemMessage(*spec, t_value);
        if (!message.empty())
        {
            m_notificationOverlay.show(message, spec->icon);
        }
    }
    return true;
}

void Mod::drawOverlay()
{
    m_notificationOverlay.draw();
    m_blipManager.drawNumbers();
    m_ammuNationShop.drawShopContents();
    m_trapHandler.drawTimers();

}

void Mod::drawMenuOverlay()
{
    // The pause menu is also where the mod's little settings live - poll the toggle here,
    // since this only runs while a menu is open.
    if (m_tagBlipToggleKey.justPressed())
    {
        m_blipManager.toggleBlips();
    }

    bool connected = m_apSocket.isConnected();
    float bottom = static_cast<float>(RsGlobal.maximumHeight);

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(ScreenScale::of(0.7f), ScreenScale::of(1.4f));
    CFont::SetColor(connected ? CRGBA(80, 220, 80, 255) : CRGBA(220, 80, 80, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);
    CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth));

    CFont::PrintString(ScreenScale::of(20.0f), bottom - ScreenScale::of(100.0f),
        connected ? "Archipelago: Connected" : "Archipelago: Disconnected");

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(ScreenScale::of(0.55f), ScreenScale::of(1.1f));
    CFont::SetColor(CRGBA(255, 255, 255, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);

    CFont::PrintString(ScreenScale::of(20.0f), bottom - ScreenScale::of(55.0f),
        m_blipManager.areBlipsEnabled() ? "F8 - Collectible blips on map: ON" : "F8 - Collectible blips on map: OFF");
}

void Mod::spawnCollectiblePickups()
{
	// One per collectible that needs a tool: paint for the tags, a camera for the snapshots.
	spawnPickupOnce(SPRAYCAN_PICKUP_POS, MODEL_SPRAYCAN, SPRAYCAN_PICKUP_AMMO);
	spawnPickupOnce(CAMERA_PICKUP_POS, MODEL_CAMERA, CAMERA_PICKUP_AMMO);
}

void Mod::spawnPickupOnce(const CVector& t_position, int t_modelId, unsigned int t_ammo)
{
	// Pickups created this way are stored in the game save's pickup pool, so spawning blindly
	// every session would stack duplicates - skip if ours (or the regeneration placeholder of
	// ours) is already in the pool.
	for (int i = 0; i < 620; ++i)
	{
		const CPickup& pickup = CPickups::aPickUps[i];
		if (pickup.m_nPickupType == PICKUP_NONE) continue;
		if (pickup.m_nModelIndex != t_modelId) continue;

		CVector pos = const_cast<CPickup&>(pickup).GetPosn();
		if (std::fabs(pos.x - t_position.x) < 2.0f && std::fabs(pos.y - t_position.y) < 2.0f)
		{
			return;
		}
	}

	CPickups::GenerateNewOne(t_position, t_modelId, PICKUP_ON_STREET, t_ammo, 0, false, nullptr);
}

void Mod::persistAndRestoreState(bool t_worldWiped)
{
	m_saveDataManager.poll();

	// The first-in-game-tick trigger covers a session whose menu ticks never ran (no sentinel
	// existed yet to observe the wipe); the wipe signal covers every load after that. Both are
	// guarded by the fresh-New-Game check: a brand new game has no last-passed mission at its
	// very first tick (any loadable save does; the intro's INITIAL mission passes long before
	// saving is even possible), and restoring another slot's data into a fresh game is the
	// failure mode that must stay impossible.
	bool firstInGameTick = !m_firstInGameTickHandled && FindPlayerPed();
	if (firstInGameTick)
	{
		m_firstInGameTickHandled = true;
	}

	bool restoreNeeded = false;
	if ((t_worldWiped || firstInGameTick) && CStats::LastMissionPassedName[0] != '\0')
	{
		restoreNeeded = m_saveDataManager.restoreFromCurrentLoadName();
	}

	if (firstInGameTick || t_worldWiped)
	{
		spawnCollectiblePickups();

		// A wiped world destroyed every object we cached, blockers included - drop the dangling
		// pointers without CWorld::Remove/delete (the objects are already gone; touching them
		// corrupts the world lists) and let the counter check respawn them.
		//
		// This must key off the wipe itself, NOT off restoreNeeded: a wipe that doesn't trigger
		// an AP restore used to leave m_blockersSpawned stuck true with no objects behind it, so
		// the spawn branch could never fire again and the player kept playing unblocked.
		m_missionBlockers.clear();
		m_blockersSpawned = false;
	}

	if (restoreNeeded)
	{
		m_notificationOverlay.show("Archipelago: Restored progress (" + m_saveDataManager.getCurrentSaveKey() + ")");

		for (PersistentState* subsystem : m_persistentSubsystems)
		{
			subsystem->load(m_saveDataManager);
		}
	}

	// Staged every tick rather than only on change, so a save triggered by anything - the player,
	// an autosave, a mission end - always writes current values with no separate dirty tracking.
	for (PersistentState* subsystem : m_persistentSubsystems)
	{
		subsystem->save(m_saveDataManager);
	}
}
