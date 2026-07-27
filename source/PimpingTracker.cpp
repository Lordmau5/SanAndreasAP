#include "PimpingTracker.h"

PimpingTracker::PimpingTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, PIMPING_TIERS)
{
}

void PimpingTracker::enforceSubmissionReward()
{
}

float PimpingTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_PIMPING_LEVEL);
}
