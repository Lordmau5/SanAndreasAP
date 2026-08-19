#include "SchoolTracker.h"
#include "ScriptGlobals.h"

SchoolTracker::SchoolTracker(int t_submissionID, const SubmissionTierSpec& t_spec,
	std::span<const int> t_scoreGlobals)
	: TieredSubmissionTracker(t_submissionID, t_spec), m_scoreGlobals(t_scoreGlobals)
{
}

void SchoolTracker::enforceSubmissionReward()
{
}

float SchoolTracker::getProgress() const
{
	int passed = 0;
	for (int test = 0; test < static_cast<int>(m_scoreGlobals.size()); ++test)
	{
		if (isTestPassed(test, ScriptGlobals::read(m_scoreGlobals[test]))) passed++;
	}
	return static_cast<float>(passed);
}

bool SchoolTracker::isTestPassed(int, int t_value) const
{
	// The stored value is a personal best, so a bad retry never drops a test back below the mark.
	return t_value >= PASS_SCORE;
}
