#pragma once
#include "TieredSubmissionTracker.h"

// Territory retaken during Return to Los Santos, paid out per 5% held. No AP reward item - the
// reward is the story progress the percentage unlocks.
class GangTerritoryTracker : public TieredSubmissionTracker
{
public:
	GangTerritoryTracker(int t_submissionID);
	void enforceSubmissionReward() override;
	void restoreSentTier(int t_tier) override;

protected:
	float getProgress() const override;

private:
	mutable bool m_armed = false;
};
