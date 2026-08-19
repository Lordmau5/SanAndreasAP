#include "GangTerritoryTracker.h"
#include <CGangWars.h>

GangTerritoryTracker::GangTerritoryTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, GANG_TERRITORY_TIERS)
{
}

void GangTerritoryTracker::enforceSubmissionReward()
{
}

float GangTerritoryTracker::getProgress() const
{
	CGangWars::UpdateTerritoryUnderControlPercentage();
	float pct = CGangWars::TerritoryUnderControlPercentage * 100.0f;

	if (pct < GANG_TERRITORY_TIERS.progressPerTier) m_armed = true;
	return m_armed ? pct : 0.0f;
}

void GangTerritoryTracker::restoreSentTier(int t_tier)
{
	TieredSubmissionTracker::restoreSentTier(t_tier);
	m_armed = t_tier > 0;
}
