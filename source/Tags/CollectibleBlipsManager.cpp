#include "CollectibleBlipsManager.h"
#include "ScreenScale.h"
#include "ModSettings.h"
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
	constexpr char SHOW_BLIPS_KEY[] = "show_tag_blips";

	constexpr float POSITION_TOLERANCE_SQ = 0.0001f;
}

void CollectibleBlipsManager::save(SaveDataManager& t_saveData)
{
	t_saveData.setValue(SHOW_BLIPS_KEY, m_blipsEnabled ? "1" : "0");
}

void CollectibleBlipsManager::load(const SaveDataManager& t_saveData)
{
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

void CollectibleBlipsManager::clearStaleSentinels()
{
	for (unsigned int i = 0; i < MAX_RADAR_TRACES; ++i)
	{
		const tRadarTrace& trace = CRadar::ms_RadarTrace[i];
		if (!trace.m_bInUse) continue;

		if (trace.m_nBlipType != BLIP_COORD) continue;
		if (trace.m_nRadarSprite != RADAR_SPRITE_NONE) continue;
		if (trace.m_nBlipDisplay != BLIP_DISPLAY_NEITHER) continue;
		if (trace.m_vecPos.x != 0.0f || trace.m_vecPos.y != 0.0f || trace.m_vecPos.z != 0.0f) continue;

		CRadar::ClearBlip(CRadar::GetNewUniqueBlipIndex(static_cast<int>(i)));
	}
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
			m_handles[index] = handle;
		}
		else
		{
			CRadar::ClearBlip(handle);
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

	if (MenuMap::isOpen())
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
		clearStaleSentinels();
		m_sentinelHandle = CRadar::SetCoordBlip(BLIP_COORD, CVector(0.0f, 0.0f, 0.0f), 0, BLIP_DISPLAY_NEITHER, nullptr);
	}

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

		if (handle != -1 && !ownsBlip(handle))
		{
			handle = -1;
		}
		int arrayIndex = handle != -1 ? CRadar::GetActualBlipArrayIndex(handle) : -1;

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
			CRadar::ms_RadarTrace[arrayIndex].m_bShortRange = !target.located;
		}
	}

	return worldWiped;
}

void CollectibleBlipsManager::drawRadarNumbers() const
{
	if (MenuMap::isOpen()) return;
	if (!m_blipsEnabled) return;

	CFont::SetFontStyle(FONT_SUBTITLES);
	const float numberScale = ModSettings::collectibleNumberScale();
	CFont::SetScale(numberScale, numberScale * 2.0f);
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_CENTER);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);

	for (int i = 0; i < static_cast<int>(m_targets.size()); ++i)
	{
		if (m_handles[i] == -1) continue;

		const BlipTarget& target = m_targets[i];

		CVector2D radarSpace;
		CVector2D worldPos(target.position.x, target.position.y);
		CRadar::TransformRealWorldPointToRadarSpace(radarSpace, worldPos);

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
	if (!MenuMap::isOpen()) return;
	if (!m_blipsEnabled) return;

	clearAllBlips();

	const float half = ScreenScale::of(16.0f);
	for (const BlipTarget& target : m_targets)
	{
		if (target.claimed) continue;

		CVector2D screenPos;
		if (!MenuMap::worldToScreen(target.position, screenPos)) continue;

		CVector2D iconPos = MenuMap::clampToMap(screenPos, half);
		CRadar::RadarBlipSprites[target.sprite].Draw(
			CRect(iconPos.x - half, iconPos.y - half, iconPos.x + half, iconPos.y + half),
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
	const float numberInset = ScreenScale::of(10.0f);
	for (const BlipTarget& target : m_targets)
	{
		if (target.claimed) continue;

		CVector2D screenPos;
		if (!MenuMap::worldToScreen(target.position, screenPos)) continue;

		CVector2D iconPos = MenuMap::clampToMap(screenPos, half);
		CVector2D numberPos = MenuMap::clampToMap(CVector2D(iconPos.x + offset, iconPos.y + offset), numberInset);
		CFont::PrintString(numberPos.x, numberPos.y, std::to_string(target.number).c_str());
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
