#pragma once
#include "TieredSubmissionTracker.h"

// The Hunter Quarry sub-missions, one check each.
class QuarryTracker : public TieredSubmissionTracker
{
public:
	QuarryTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;
};
