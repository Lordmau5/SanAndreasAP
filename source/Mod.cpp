#include "Mod.h"
#include "PlayerControl.h"
#include "APProtocol.h"
#include "ItemEffects.h"
#include "CStreaming.h"
#include "CPools.h"
#include <CRadar.h>
#include <CTimer.h>

Mod::Mod()
{
	m_apSocket.connectToServer("127.0.0.1", 12345);

	m_persistentSubsystems = { &m_checkListener, &m_checkGiver, &m_blipManager, &m_receivedItemLog, &m_trapHandler };

	// Staged here rather than every tick, so a save started mid-tick still records current values.
	GameStorageHook::setBeforeSaveCallback([this]
	{
		// No save may contain them - they cost 68 of the 350 object-pool slots and stack on reload.
		removeMissionBlockers();

		for (PersistentState* subsystem : m_persistentSubsystems)
		{
			subsystem->save(m_saveDataManager);
		}
		m_saveDataManager.recordValuesAtSave();
	});
}

void Mod::start()
{
    // The order of these phases is load-bearing; each one's comment says why.
    m_apSocket.update();
    pollDeathLink();

    // Consumed once, then handed to everything that needs it - the blip manager included.
    bool loadHooked = GameStorageHook::consumeLoadHappened();

    if (GameStorageHook::consumeNewGameHappened())
    {
        resetForNewGame();
    }
    else if (loadHooked)
    {
        // A load restores its own weapons rather than clearing them, so it cancels any pending
        // New Game re-grant delay instead of making the loaded save's items wait it out.
        m_newGameRegrantPending = false;
    }

    bool worldWiped = updateWorldState(loadHooked);
    persistAndRestoreState(worldWiped, loadHooked);

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
bool Mod::updateWorldState(bool t_loadHooked)
{
    // Loads come from the hook; the blip pool is left only to catch a New Game clearing the radar.
    if (t_loadHooked)
    {
        m_blipManager.onWorldWiped();
    }
    return m_blipManager.render(collectBlipTargets());
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

        // Saves from before they were kept out can hold stacked sets - one set is all that is ever
        // wanted, and the surplus alone can exhaust the pool.
        if (m_missionBlockers.size() > missionStartPos.size() * 2)
        {
            removeMissionBlockers();
        }
    }

    bool blocked = m_checkGiver.getProgressiveMissionCounter() == 0;

    if (blocked && !m_blockersSpawned)
    {
        spawnMissionBlockers();
    }
    else if (!blocked && m_blockersSpawned)
    {
        removeMissionBlockers();
    }

    // Tied to running out, not to spawning - blockers also come and go around every save.
    if (blocked && !m_outOfMissionsNotified)
    {
        m_notificationOverlay.show("Note: You are out of Progressive Missions. Missions will be blocked until you unlock more.");
        m_outOfMissionsNotified = true;
    }
    else if (!blocked)
    {
        m_outOfMissionsNotified = false;
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

bool Mod::hasBlockerAt(const Position& t_spawn, int t_modelId) const
{
    float expectedZ = t_modelId == BARRICADE_MODEL_ID ? t_spawn.z + BARRICADE_Z_OFFSET : t_spawn.z;

    for (CObject* blocker : m_missionBlockers)
    {
        if (!blocker || blocker->m_nModelIndex != t_modelId) continue;

        CVector position = blocker->GetPosition();
        float dx = position.x - t_spawn.x;
        float dy = position.y - t_spawn.y;
        float dz = position.z - expectedZ;
        if (dx * dx + dy * dy + dz * dz < BLOCKER_POSITION_TOLERANCE_SQ) return true;
    }
    return false;
}

void Mod::spawnMissionBlockers()
{
    // Own whatever is already there before creating more, or an old save's set gets doubled.
    adoptExistingBlockers();

    CStreaming::RequestModel(BLOCKER_MODEL_ID, 0);
    CStreaming::RequestModel(BARRICADE_MODEL_ID, 0);
    CStreaming::LoadAllRequestedModels(false);

    for (const Position& pos : missionStartPos) {
        CObject* blocker = hasBlockerAt(pos, BLOCKER_MODEL_ID) ? nullptr : CObject::Create(BLOCKER_MODEL_ID);

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

        CObject* barricade = hasBlockerAt(pos, BARRICADE_MODEL_ID) ? nullptr : CObject::Create(BARRICADE_MODEL_ID);

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
        if (!object) continue;

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

    // A New Game's script init strips CJ's weapons and stats a beat after he spawns. Hold the
    // re-grant until the player has held control CONTINUOUSLY a little past that init, so the grants
    // land after the wipe rather than getting cleared by it.
    if (m_newGameRegrantPending)
    {
        if (!PlayerControl::isInControl())
        {
            // The intro cutscene (and a flicker of control before it) must not run the timer down,
            // or the grant lands right as the wipe fires on skip. Any break restarts the wait.
            m_newGameRegrantClockStarted = false;
            return;
        }
        unsigned int now = CTimer::m_snTimeInMilliseconds;
        // Re-anchor on a backwards jump too: the game resets its timer around the new game.
        if (!m_newGameRegrantClockStarted || now < m_newGameRegrantControlStartMs)
        {
            m_newGameRegrantClockStarted = true;
            m_newGameRegrantControlStartMs = now;
            return;
        }
        if (now - m_newGameRegrantControlStartMs < NEW_GAME_REGRANT_DELAY_MS) return;
        m_newGameRegrantPending = false;
    }

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

void Mod::persistAndRestoreState(bool t_worldWiped, bool t_loadHooked)
{
	m_saveDataManager.poll();

	bool firstInGameTick = FindPlayerPed() && !m_firstInGameTickHandled;
	if (firstInGameTick)
	{
		m_firstInGameTickHandled = true;
	}

	// Only the hook may trigger this. The wipe signal cannot tell a load from a save, and a
	// "first tick with a last-passed mission" test fires on a New Game's first mission instead.
	bool restoreNeeded = false;
	if (t_loadHooked && CStats::LastMissionPassedName[0] != '\0')
	{
		restoreNeeded = m_saveDataManager.restoreFromCurrentLoadName();
	}

	// Still keyed off the wipe: a save destroys our objects too, so the cached pointers dangle.
	if (firstInGameTick || t_worldWiped || t_loadHooked)
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

}

void Mod::resetForNewGame()
{
	// Feed every subsystem an empty manager so each returns to its fresh-game defaults through the
	// same path a load uses. This can't restore another slot's data - the values are empty by
	// construction - and it resets m_lastAppliedIndex with the rest, so every item the session has
	// already received is re-granted to the new save on the next tick rather than lost to a softlock.
	SaveDataManager freshDefaults;
	for (PersistentState* subsystem : m_persistentSubsystems)
	{
		subsystem->load(freshDefaults);
	}

	m_blipManager.onWorldWiped();
	m_missionBlockers.clear();
	m_blockersSpawned = false;

	m_firstInGameTickHandled = false;
	m_outOfMissionsNotified = false;
	m_newGameRegrantPending = true;
	m_newGameRegrantClockStarted = false;
}
