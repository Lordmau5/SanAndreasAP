#include "DrivingSchoolTracker.h"
#include "ScriptGlobals.h"

namespace
{
	// Each test's best percentage, in its own hand-allocated global - neither contiguous nor in test
	// order, so they are listed rather than derived. Read in preference to the furthest-unlocked-test
	// global ($53), which stops moving once test 12 unlocks and so can never see it passed.
	constexpr int TEST_SCORE_GLOBALS[] = { 91, 92, 94, 96, 97, 98, 100, 101, 102, 103, 105, 107 };

	// The school's pass mark. Comparing against it rather than zero keeps a failed run from counting.
	constexpr int BRONZE_SCORE = 70;
}

DrivingSchoolTracker::DrivingSchoolTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, DRIVING_SCHOOL_TIERS)
{
}

void DrivingSchoolTracker::enforceSubmissionReward()
{
}

float DrivingSchoolTracker::getProgress() const
{
	int passed = 0;
	for (int scoreGlobal : TEST_SCORE_GLOBALS)
	{
		if (ScriptGlobals::read(scoreGlobal) >= BRONZE_SCORE) passed++;
	}
	return static_cast<float>(passed);
}
