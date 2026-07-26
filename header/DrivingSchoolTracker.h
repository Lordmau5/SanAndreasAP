#pragma once
#include "TieredSubmissionTracker.h"

// Driving School in Doherty: 12 tests, one check each, at any medal - bronze is the game's own
// pass mark. Tests unlock in order, so "tests passed" is monotonic and the tier model applies
// directly. Back to School stays a separate mission location - it fires on passing all 12.
//
// No AP reward item: the school pays out in vehicles and a Wang Cars unlock, not a player ability.
class DrivingSchoolTracker : public TieredSubmissionTracker
{
public:
	DrivingSchoolTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;
};
