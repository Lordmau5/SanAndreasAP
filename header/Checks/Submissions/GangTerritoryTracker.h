#pragma once
#include "TieredSubmissionTracker.h"

class GangTerritoryTracker : public TieredSubmissionTracker
{
public:
	GangTerritoryTracker(int t_submissionID);
	void enforceSubmissionReward() override;
	void restoreSentState(const std::string& t_state) override;

protected:
	float getProgress() const override;

private:
	mutable bool m_armed = false;
};
