#include "TagTracker.h"
#include "common.h"
#include <CTheScripts.h>
#include <eWeaponType.h>
#include "TagPositions.h"
#include "common.h"
#include <CRadar.h>
#include <cstdint>

TagTracker::TagTracker()
	: Collectible<100>(tagPositions, RADAR_SPRITE_SPRAY, "tags_claimed", "TAG")
{
}

float TagTracker::readCount() const
{
	return static_cast<float>(*reinterpret_cast<int32_t*>(TAGS_SPRAYED_ADDR));
}

int TagTracker::identifyCollected() const
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return -1;

	CVector playerPos = player->GetPosition();

	int best = -1;
	float bestDistance = 0.0f;
	for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
	{
		float distance = CVector::Distance(playerPos, tagPositions[i]);
		if (best == -1 || distance < bestDistance)
		{
			best = i;
			bestDistance = distance;
		}
	}
	return best;
}

bool TagTracker::update()
{
	if (!isUnlocked() && !CTheScripts::IsPlayerOnAMission())
	{
		if (CPlayerPed* player = FindPlayerPed())
		{
			int slot = player->GetWeaponSlot(WEAPONTYPE_SPRAYCAN);
			if (slot >= 0 && player->m_aWeapons[slot].m_eWeaponType == WEAPONTYPE_SPRAYCAN)
			{
				player->ClearWeapon(WEAPONTYPE_SPRAYCAN);
				m_noticePending = true;
			}
		}
	}

	return Collectible<100>::update();
}

const char* TagTracker::consumeLockedNotice()
{
	if (!m_noticePending) return nullptr;

	m_noticePending = false;
	return "Archipelago: Spray can removed - Tags are locked";
}
