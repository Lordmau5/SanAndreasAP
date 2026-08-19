#pragma once
#include "SchoolTracker.h"

class DrivingSchoolTracker : public SchoolTracker
{
public:
	DrivingSchoolTracker(int t_submissionID);

	void enforceSubmissionReward() override;
};
