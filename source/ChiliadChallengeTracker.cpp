#include "ChiliadChallengeTracker.h"
#include "ScriptGlobals.h"

ChiliadChallengeTracker::ChiliadChallengeTracker(int t_locationId, int t_completedGlobal)
	: SubmissionTracker(t_locationId), m_completedGlobal(t_completedGlobal)
{
}

void ChiliadChallengeTracker::enforceSubmissionReward()
{
}

bool ChiliadChallengeTracker::pollCompletion()
{
	if (submissionCompleted) return false;

	return ScriptGlobals::read(m_completedGlobal) == 1;
}
