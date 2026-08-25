#include "SubmissionStartBlocked.h"
#include "Submissions/SubmissionTracker.h"
#include "ScriptCommandHook.h"
#include "EntityIDs.h"
#include <CRunningScript.h>
#include <eScriptCommands.h>
#include <cstring>

namespace
{
	const std::vector<std::unique_ptr<SubmissionTracker>>* g_trackers = nullptr;

	bool vigilanteLocked()
	{
		if (!g_trackers) return false;

		for (const auto& tracker : *g_trackers)
		{
			if (tracker->getSubmissionID() != VIGILANTE_ID) continue;
			return !tracker->vehiclesUnlocked();
		}
		return false;
	}

	bool blockPoliceVehicleTest(CRunningScript* t_script)
	{
		return _strnicmp(t_script->m_szName, "R3", 8) == 0 && vigilanteLocked();
	}
}

void SubmissionStartBlocked::install(const std::vector<std::unique_ptr<SubmissionTracker>>& t_trackers)
{
	g_trackers = &t_trackers;

	ScriptCommandHook::blockCommand(COMMAND_IS_CHAR_IN_ANY_POLICE_VEHICLE, &blockPoliceVehicleTest);
}
