#pragma once
#include "TieredSubmissionTracker.h"
#include <CStats.h>

class PimpingTracker : public TieredSubmissionTracker
{
public:
	PimpingTracker(int t_submissionID);
	void enforceSubmissionReward() override;
	bool locksVehicleModel(int t_modelId) const override;
	bool isVehicleValid(int t_modelId) const override;

protected:
	float getProgress() const override;
};
