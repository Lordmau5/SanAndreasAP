#pragma once
#include "SubmissionTracker.h"

class ChiliadChallengeTracker : public SubmissionTracker
{
public:
	ChiliadChallengeTracker(int t_locationId, int t_completedGlobal);

	void enforceSubmissionReward() override;
	bool pollCompletion() override;

private:
	const int m_completedGlobal;
};
