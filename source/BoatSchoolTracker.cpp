#include "BoatSchoolTracker.h"

namespace
{
	// In the school's test order: milliseconds for the timed tests, whole metres for the jump.
	constexpr int PASS_LIMITS[] = { 12000, 40000, 120000, 55, 180000 };

	// Flying Fish, the only test where a bigger number is better.
	constexpr int JUMP_TEST = 3;
}

BoatSchoolTracker::BoatSchoolTracker(int t_submissionID)
	: SchoolTracker(t_submissionID, BOAT_SCHOOL_TIERS, BOAT_SCHOOL_SCORE_GLOBALS)
{
}

bool BoatSchoolTracker::isTestPassed(int t_testIndex, int t_value) const
{
	if (t_testIndex < 0 || t_testIndex >= static_cast<int>(std::size(PASS_LIMITS))) return false;

	if (t_testIndex == JUMP_TEST) return t_value > PASS_LIMITS[t_testIndex];

	// Unattempted tests hold a high sentinel, so "under the limit" means passed - but zero is what
	// the globals read before the scripts write them, and that is not a pass.
	return t_value > 0 && t_value < PASS_LIMITS[t_testIndex];
}
