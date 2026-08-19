#pragma once
#include "TieredSubmissionTracker.h"

class CourierTracker : public TieredSubmissionTracker
{
public:
	CourierTracker(int t_submissionID, const SubmissionTierSpec& t_spec, int t_cityId, int t_passedGlobal);

	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;

private:
	bool isCourierScriptActive() const;

	const int m_cityId;
	const int m_passedGlobal;
};
