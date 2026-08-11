#include "CollectibleBlipsManager.h"
#include "ScreenScale.h"
#include "MenuMap.h"
#include "SaveDataManager.h"
#include "common.h"
#include <algorithm>
#include <string>
#include <utility>
#include <CRadar.h>
#include <CFont.h>
#include <CRGBA.h>
#include <CRect.h>
#include <CSprite2d.h>
#include <CMenuManager.h>

namespace
{
	// Kept as the old key so existing saves' toggle preference still loads.
	constexpr char SHOW_BLIPS_KEY[] = "show_tag_blips";

	constexpr float POSITION_TOLERANCE_SQ = 0.0001f;

	bool isPauseMapOpen()
	{
		return FrontEndMenuManager.m_bMenuActive && FrontEndMenuManager.m_nCurrentMenuPage == MENUPAGE_MAP;
	}
}

void CollectibleBlipsManager::save(SaveDataManager& t_saveData)
{
	t_saveData.setValue(SHOW_BLIPS_KEY, m_blipsEnabled ? "1" : "0");
}

void CollectibleBlipsManager::load(const SaveDataManager& t_saveData)
{
	// Defaults to on: a save from before the toggle existed should show blips, since that was the
	// only behaviour available when it was written.
	m_blipsEnabled = t_saveData.getValue(SHOW_BLIPS_KEY, "1") == "1";
}

int CollectibleBlipsManager::targetIndexAt(const CVector& t_pos) const
{
	for (int i = 0; i < static_cast<int>(m_targets.size()); ++i)
	{
		float dx = t_pos.x - m_targets[i].position.x;
		float dy = t_pos.y - m_targets[i].position.y;
		if (dx * dx + dy * dy < POSITION_TOLERANCE_SQ) return i;
	}
	return -1;
}

int CollectibleBlipsManager::ownedTargetIndexAt(const CVector& t_pos, int t_sprite) const
{
	int index = targetIndexAt(t_pos);
	if (index < 0) return -1;
	if (m_targets[index].sprite != t_sprite) return -1;

	return index;
}

// A stale handle can still resolve after a load - to whatever blip the save put in that slot. So
// never trust the handle alone; clearing the game's mission blip is permanent.
bool CollectibleBlipsManager::ownsBlip(int t_handle) const
{
	if (t_handle == -1) return false;

	int arrayIndex = CRadar::GetActualBlipArrayIndex(t_handle);
	if (arrayIndex < 0) return false;

	const tRadarTrace& trace = CRadar::ms_RadarTrace[arrayIndex];
	return ownedTargetIndexAt(trace.m_vecPos, trace.m_nRadarSprite) >= 0;
}

int CollectibleBlipsManager::findExistingBlipAt(const CVector& t_pos, int t_sprite) const
{
	for (unsigned int i = 0; i < MAX_RADAR_TRACES; ++i)
	{
		const tRadarTrace& trace = CRadar::ms_RadarTrace[i];
		if (!trace.m_bInUse) continue;
		if (trace.m_nRadarSprite != t_sprite) continue;

		float dx = t_pos.x - trace.m_vecPos.x;
		float dy = t_pos.y - trace.m_vecPos.y;
		if (dx * dx + dy * dy < POSITION_TOLERANCE_SQ)
		{
			return CRadar::GetNewUniqueBlipIndex(static_cast<int>(i));
		}
	}
	return -1;
}

void CollectibleBlipsManager::reconcileWithPool()
{
	std::fill(m_handles.begin(), m_handles.end(), -1);

	for (unsigned int i = 0; i < MAX_RADAR_TRACES; ++i)
	{
		const tRadarTrace& trace = CRadar::ms_RadarTrace[i];
		if (!trace.m_bInUse) continue;

		int index = ownedTargetIndexAt(trace.m_vecPos, trace.m_nRadarSprite);
		if (index < 0) continue;

		int handle = CRadar::GetNewUniqueBlipIndex(static_cast<int>(i));
		if (m_handles[index] == -1)
		{
			m_handles[index] = handle; // adopt the one the save restored
		}
		else
		{
			CRadar::ClearBlip(handle); // duplicate from an earlier load - drop it
		}
	}
}

void CollectibleBlipsManager::onWorldWiped()
{
	m_sentinelHandle = -1;
	m_reconcileTicks = RECONCILE_TICKS;
}

bool CollectibleBlipsManager::render(std::vector<BlipTarget> t_targets)
{
	m_targets = std::move(t_targets);
	m_handles.resize(m_targets.size(), -1);

	if (isPauseMapOpen())
	{
		clearAllBlips();
		return false;
	}

	bool worldWiped = false;
	if (m_sentinelHandle != -1 && CRadar::GetActualBlipArrayIndex(m_sentinelHandle) < 0)
	{
		worldWiped = true;
		m_sentinelHandle = -1;
	}
	if (m_sentinelHandle == -1)
	{
		m_sentinelHandle = CRadar::SetCoordBlip(BLIP_COORD, CVector(0.0f, 0.0f, 0.0f), 0, BLIP_DISPLAY_NEITHER, nullptr);
	}

	// Our blips are written into the save file, so any load - including one straight after launching
	// the game, where there is no previous world to detect a wipe against - brings a full set back.
	// Creating another set on top doubles the count each time, and the radar pool is only 175
	// traces; once it is full the game can no longer create ITS OWN blips, so mission markers
	// silently stop appearing. Reconciling against what is actually in the world makes this
	// idempotent, and repairs saves that already accumulated duplicates.
	if (m_reconcileTicks > 0 && FindPlayerPed())
	{
		reconcileWithPool();
		m_reconcileTicks--;
	}

	if (!m_blipsEnabled)
	{
		for (int& handle : m_handles)
		{
			if (ownsBlip(handle))
			{
				CRadar::ClearBlip(handle);
			}
			handle = -1;
		}
		return worldWiped;
	}

	const float radarRange = CRadar::m_radarRange;

	for (int i = 0; i < static_cast<int>(m_targets.size()); ++i)
	{
		const BlipTarget& target = m_targets[i];
		int& handle = m_handles[i];

		// Drop any handle we can't prove still refers to one of our own blips.
		if (handle != -1 && !ownsBlip(handle))
		{
			handle = -1;
		}
		int arrayIndex = handle != -1 ? CRadar::GetActualBlipArrayIndex(handle) : -1;

		// The located tag ignores the ranking - it is the one the player asked to follow, so it
		// stays shown however far away. A tag already showing keeps its blip a little past the
		// cut-off, so one sitting right on the boundary doesn't flicker as the player moves.
		bool wantBlip = false;
		if (!target.claimed)
		{
			const float keepRange = handle != -1 ? radarRange * RANGE_HYSTERESIS : radarRange;

			if (target.located) wantBlip = true;
			else if (target.distance <= keepRange && target.rank < MAX_BLIPS) wantBlip = true;
		}

		if (!wantBlip)
		{
			if (handle != -1)
			{
				CRadar::ClearBlip(handle);
				handle = -1;
			}
			continue;
		}

		if (handle == -1)
		{
			// Blips persist in save files, so this entry's blip may already be in the world. Adopt
			// it instead of adding a second one - duplicates are what exhaust the 175-trace pool and
			// stop the game creating its own mission-marker blips. Checking here rather than only on
			// load makes it immune to when the save's radar data actually lands.
			handle = findExistingBlipAt(target.position, target.sprite);
			if (handle == -1)
			{
				handle = CRadar::SetCoordBlip(BLIP_COORD, target.position, 0, BLIP_DISPLAY_BOTH, nullptr);
				if (handle != -1 && CRadar::GetActualBlipArrayIndex(handle) >= 0)
				{
					CRadar::SetBlipSprite(handle, target.sprite);
				}
				else
				{
					handle = -1;
				}
			}
			arrayIndex = handle != -1 ? CRadar::GetActualBlipArrayIndex(handle) : -1;
		}

		if (arrayIndex >= 0)
		{
			// Short-range blips (the mechanism shop icons use) still show on the pause map but only
			// appear on the radar when the player is close - without this, every entry clamps to the
			// radar's edge and buries it in icons. The located tag stays full-range on purpose,
			// making it the only edge-clamped icon to follow.
			CRadar::ms_RadarTrace[arrayIndex].m_bShortRange = !target.located;
		}
	}

	return worldWiped;
}

void CollectibleBlipsManager::drawNumbers() const
{
	if (!m_blipsEnabled) return;

	CFont::SetFontStyle(FONT_SUBTITLES);
	CFont::SetScale(ScreenScale::of(0.3f), ScreenScale::of(0.6f));
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_CENTER);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);

	for (int i = 0; i < static_cast<int>(m_targets.size()); ++i)
	{
		// Only entries that actually have a blip get a number, so a number can never appear with no
		// icon underneath it now that distant ones aren't blipped.
		if (m_handles[i] == -1) continue;

		const BlipTarget& target = m_targets[i];

		CVector2D radarSpace;
		CVector2D worldPos(target.position.x, target.position.y);
		CRadar::TransformRealWorldPointToRadarSpace(radarSpace, worldPos);

		// Skip entries outside the visible radar area instead of clamping them to the edge the way
		// native blips do. The radar is a circle of radius 1 in radar space, so clip against that
		// (slightly inside it, since the text has its own extent) rather than the enclosing square -
		// corner positions would render outside the visible disc.
		if (radarSpace.x * radarSpace.x + radarSpace.y * radarSpace.y > 0.85f * 0.85f) continue;

		CVector2D screenPos;
		CRadar::TransformRadarPointToScreenSpace(screenPos, radarSpace);
		CFont::PrintString(screenPos.x, screenPos.y, std::to_string(target.number).c_str());
	}
}

void CollectibleBlipsManager::clearAllBlips()
{
	for (unsigned int i = 0; i < MAX_RADAR_TRACES; ++i)
	{
		const tRadarTrace& trace = CRadar::ms_RadarTrace[i];
		if (!trace.m_bInUse) continue;
		if (ownedTargetIndexAt(trace.m_vecPos, trace.m_nRadarSprite) < 0) continue;
		CRadar::ClearBlip(CRadar::GetNewUniqueBlipIndex(static_cast<int>(i)));
	}
	std::fill(m_handles.begin(), m_handles.end(), -1);
}

void CollectibleBlipsManager::drawMapOverlay()
{
	if (!isPauseMapOpen()) return;
	if (!m_blipsEnabled) return;

	clearAllBlips();

	const float half = ScreenScale::of(16.0f);
	for (const BlipTarget& target : m_targets)
	{
		if (target.claimed) continue;

		CVector2D screenPos;
		if (!MenuMap::worldToScreen(target.position, screenPos)) continue;

		CRadar::RadarBlipSprites[target.sprite].Draw(
			CRect(screenPos.x - half, screenPos.y - half, screenPos.x + half, screenPos.y + half),
			CRGBA(255, 255, 255, 255));
	}

	const float scale = ScreenScale::of(0.5f);
	CFont::SetFontStyle(FONT_SUBTITLES);
	CFont::SetScale(scale, scale * 2.0f);
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_CENTER);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);

	const float offset = ScreenScale::of(7.0f);
	for (const BlipTarget& target : m_targets)
	{
		if (target.claimed) continue;

		CVector2D screenPos;
		if (!MenuMap::worldToScreen(target.position, screenPos)) continue;

		CFont::PrintString(screenPos.x + offset, screenPos.y + offset, std::to_string(target.number).c_str());
	}
}

void CollectibleBlipsManager::toggleBlips()
{
	m_blipsEnabled = !m_blipsEnabled;
}

bool CollectibleBlipsManager::areBlipsEnabled() const
{
	return m_blipsEnabled;
}
