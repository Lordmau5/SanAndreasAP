#pragma once
#include "SchoolTracker.h"

// Boat School is the one school not scored on percentage: four tests are timed and one is a jump
// distance, so only the per-test pass rule differs from the others.
class BoatSchoolTracker : public SchoolTracker
{
public:
	BoatSchoolTracker(int t_submissionID);

protected:
	bool isTestPassed(int t_testIndex, int t_value) const override;
};
