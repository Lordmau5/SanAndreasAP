#include "BurglaryTracker.h"

BurglaryTracker::BurglaryTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, BURGLARY_TIERS)
{
}

void BurglaryTracker::enforceSubmissionReward()
{
	if (checkReceived)
	{
		CWorld::Players[0].m_bDoesNotGetTired = true;
	}
	else if (submissionCompleted)
	{
		CWorld::Players[0].m_bDoesNotGetTired = false;
	}
}

float BurglaryTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_MONEY_MADE_FROM_BURGLARY);
}
