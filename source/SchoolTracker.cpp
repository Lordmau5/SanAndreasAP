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
	for (int scoreGlobal : m_scoreGlobals)
	{
		// Comparing against the pass mark rather than zero keeps a failed run from counting, and the
		// stored value is a personal best, so a bad retry never drops a test back below the line.
		if (ScriptGlobals::read(scoreGlobal) >= PASS_SCORE) passed++;
	}
	return static_cast<float>(passed);
}
