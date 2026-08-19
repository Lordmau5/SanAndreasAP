#pragma once
#include "SchoolTracker.h"

class BikeSchoolTracker : public SchoolTracker
{
public:
	BikeSchoolTracker(int t_submissionID);

	void enforceSubmissionReward() override;
};
