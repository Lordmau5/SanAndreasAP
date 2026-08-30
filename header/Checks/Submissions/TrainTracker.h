#pragma once
#include "TieredSubmissionTracker.h"
#include "ScriptGlobals.h"

class TrainTracker : public TieredSubmissionTracker
{
public:
	TrainTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;
	int currentTier() const override;
};

