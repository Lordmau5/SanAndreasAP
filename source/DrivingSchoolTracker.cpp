#include "DrivingSchoolTracker.h"
#include "ScriptGlobals.h"

namespace
{
	// Each test's best percentage lives in its own hand-allocated global - the slots are neither
	// contiguous nor in test order, so they are listed rather than derived. Confirmed by matching
	// all twelve values against a fully completed school.
	//
	// The count is taken from these rather than from the school's furthest-unlocked-test global,
	// which stops moving once the twelfth test unlocks and so can never see it passed.
	constexpr int TEST_SCORE_GLOBALS[] = { 91, 92, 94, 96, 97, 98, 100, 101, 102, 103, 105, 107 };

	// Bronze, the school's pass mark. Testing against it rather than against zero keeps a failed
	// run from counting even if the game records its score, and the stored value is a personal
	// best, so a bad retry can never drop a test back below the line.
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
