#pragma once
#include "SubmissionTracker.h"

class GlobalFlagTracker : public SubmissionTracker
{
public:
	GlobalFlagTracker(int t_locationId, int t_completedGlobal);

	void enforceSubmissionReward() override;
	bool pollCompletion() override;

private:
	const int m_completedGlobal;
};
