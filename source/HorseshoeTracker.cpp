#include "HorseshoeTracker.h"
#include "HorseshoePositions.h"
#include "common.h"
#include <CRadar.h>
#include <CStats.h>

HorseshoeTracker::HorseshoeTracker()
	: Collectible<50>(horseshoePositions, RADAR_SPRITE_CASH, "horseshoes_claimed", "HORSESHOE")
{
}

float HorseshoeTracker::readCount() const
{
	return CStats::GetStatValue(STAT_HORSESHOES_COLLECTED);
}

int HorseshoeTracker::identifyCollected() const
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return -1;

	CVector playerPos = player->GetPosition();

	int best = -1;
	float bestDistance = 0.0f;
	for (int i = 0; i < static_cast<int>(horseshoePositions.size()); ++i)
	{
		if (isClaimed(i)) continue;

		float distance = CVector::Distance(playerPos, horseshoePositions[i]);
		if (best == -1 || distance < bestDistance)
		{
			best = i;
			bestDistance = distance;
		}
	}
	return best;
}
