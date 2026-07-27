#pragma once
#include "TieredSubmissionTracker.h"
#include <CStats.h>

// Pimping, started from any pimpmobile: 10 levels, one check each. No AP reward item - finishing it
// is a money perk rather than a player ability.
class PimpingTracker : public TieredSubmissionTracker
{
public:
	PimpingTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;
};
