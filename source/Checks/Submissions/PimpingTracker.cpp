#include "PimpingTracker.h"
#include <eModelID.h>

PimpingTracker::PimpingTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, {MODEL_BROADWAY}, PIMPING_TIERS)
{
}

void PimpingTracker::enforceSubmissionReward()
{
}

float PimpingTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_PIMPING_LEVEL);
}

bool PimpingTracker::locksVehicleModel(int t_modelId) const
{
	return t_modelId == MODEL_BROADWAY;
}
