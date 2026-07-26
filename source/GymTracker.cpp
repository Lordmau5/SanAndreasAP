#include "GymTracker.h"
#include "common.h"
#include "CTheScripts.h"
#include <cstring>

GymTracker::GymTracker(int t_submissionID, eFightingStyle t_taughtStyle, const char* t_scriptName,
	FightingStyleArbiter& t_arbiter)
	: SubmissionTracker(t_submissionID), m_taughtStyle(t_taughtStyle), m_scriptName(t_scriptName)
	, m_arbiter(t_arbiter)
{
}

namespace
{
	// The fighting-style slot also holds weapon-melee styles (8+) while a melee weapon is equipped;
	// those are the game's to manage, so a reward only ever overwrites standard or another gym style.
	bool isOverwritableStyle(eFightingStyle t_style)
	{
		return t_style == STYLE_STANDARD || t_style == STYLE_BOXING
			|| t_style == STYLE_KUNG_FU || t_style == STYLE_KNEE_HEAD;
	}
}

bool GymTracker::isGymScriptActive() const
{
	for (CRunningScript* script = CTheScripts::pActiveScripts; script; script = script->m_pNext)
	{
		// Script names are 7 chars + null; the gym scripts ("gymls", "gymsf", "gymlv") all fit.
		if (_strnicmp(script->m_szName, m_scriptName, 8) == 0) return true;
	}
	return false;
}

void GymTracker::enforceSubmissionReward()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	if (isGymScriptActive() && !submissionCompleted)
	{
		if (!m_detectionArmed)
		{
			if (player->m_nFightingStyle == m_taughtStyle)
			{
				player->m_nFightingStyle = STYLE_STANDARD;
			}
			m_detectionArmed = true;
		}
		return;
	}
	m_detectionArmed = false;

	// Recency bookkeeping. Claim an order the first tick we observe our check received (covers both
	// a live receipt and a save restore); drop it if the check is later undone by an older save, so
	// a re-receipt claims a fresh, winning order.
	if (!checkReceived)
	{
		m_receiptOrder = 0;
	}
	else if (m_receiptOrder == 0)
	{
		m_receiptOrder = m_arbiter.claim();
	}

	// Only the gym that owns the style slot enforces, so the trackers never fight over it. Its style
	// overwrites standard or any OTHER earned gym style - so a newly received style replaces an
	// earlier one - but leaves weapon-melee styles alone.
	bool ownsStyleSlot = checkReceived && m_receiptOrder == m_arbiter.latest();
	if (ownsStyleSlot
		&& player->m_nFightingStyle != m_taughtStyle
		&& isOverwritableStyle(player->m_nFightingStyle))
	{
		player->m_nFightingStyle = m_taughtStyle;
	}

	// Rollback: our style is still on CJ but our check no longer is (an older save was loaded) -
	// strip it back to standard.
	if (!checkReceived && submissionCompleted && player->m_nFightingStyle == m_taughtStyle)
	{
		player->m_nFightingStyle = STYLE_STANDARD;
	}
}

// TEMPORARY
std::string GymTracker::debugState() const
{
	std::string state = std::string(m_scriptName) + "(teach=" + std::to_string(static_cast<int>(m_taughtStyle)) + ")";
	if (isGymScriptActive()) state += " IN";
	if (m_detectionArmed) state += " armed";
	if (submissionCompleted) state += " done";
	return state;
}

bool GymTracker::pollCompletion()
{
	if (submissionCompleted || !m_detectionArmed) return false;
	if (!isGymScriptActive()) return false;

	CPlayerPed* player = FindPlayerPed();
	if (!player) return false;

	return player->m_nFightingStyle == m_taughtStyle;
}
