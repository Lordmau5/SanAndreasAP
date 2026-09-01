#pragma once
#include "TieredSubmissionTracker.h"
#include <CStats.h>
#include <CWorld.h>

class ParamedicTracker : public TieredSubmissionTracker
{
public:
	ParamedicTracker(int t_submissionID);
	void enforceSubmissionReward() override;
	bool isVehicleValid(int t_modelId) const override;

protected:
	float getProgress() const override;
};

