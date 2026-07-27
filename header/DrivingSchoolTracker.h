#pragma once
#include "TieredSubmissionTracker.h"

// Driving School in Doherty: 12 tests, one check each, at any medal. Back to School stays a separate
// mission location. No AP reward item - the school pays out in vehicles, not a player ability.
class DrivingSchoolTracker : public TieredSubmissionTracker
{
public:
	DrivingSchoolTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;
};
