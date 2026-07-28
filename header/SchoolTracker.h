#pragma once
#include "TieredSubmissionTracker.h"
#include <span>

// Driving and flying school both keep one best percentage per test in a script global and both pass
// at 70%, so only the slot list and the tier spec differ. Neither has an AP reward item - they pay
// out in vehicles and story progress, not player abilities.
class SchoolTracker : public TieredSubmissionTracker
{
public:
	SchoolTracker(int t_submissionID, const SubmissionTierSpec& t_spec,
		std::span<const int> t_scoreGlobals);
	void enforceSubmissionReward() override;

protected:
	// Counts the tests that passed. Which is left to isTestPassed.
	float getProgress() const override;

	// Whether one test's stored value counts as a pass. Defaults to the game's own 70% mark, which
	// covers every school scored on percentage - boat school scores on time and distance instead.
	virtual bool isTestPassed(int t_testIndex, int t_value) const;

private:
	// The game's own pass mark, stated in its flight school text.
	static constexpr int PASS_SCORE = 70;

	std::span<const int> m_scoreGlobals;
};
