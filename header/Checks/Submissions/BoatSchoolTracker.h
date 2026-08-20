#pragma once
#include "SchoolTracker.h"

class BoatSchoolTracker : public SchoolTracker
{
public:
	BoatSchoolTracker(int t_submissionID);

protected:
	TestMedal medalForTest(int t_testIndex) const override;
};
