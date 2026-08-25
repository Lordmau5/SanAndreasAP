#include "SubmissionStartBlocked.h"
#include "Submissions/SubmissionTracker.h"
#include "ScriptCommandHook.h"
#include "EntityIDs.h"
#include <CRunningScript.h>
#include <CTheScripts.h>
#include <eScriptCommands.h>

namespace
{
	constexpr int R3_SCRIPT_BASE_OFFSET = 76402;
	constexpr int R3_VIGILANTE_POLICE_TEST_OFFSET = 1607;
	constexpr int R3_VIGILANTE_HUNTER_TEST_OFFSET = 1612;
	constexpr int OPCODE_SIZE = 2;

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

	bool isAtInstruction(CRunningScript* t_script, int t_offset)
	{
		unsigned char* instruction = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace)
			+ R3_SCRIPT_BASE_OFFSET + t_offset + OPCODE_SIZE;

		return t_script->m_pCurrentIP == instruction;
	}

	bool blockVigilanteGate(CRunningScript* t_script)
	{
		if (!vigilanteLocked()) return false;

		return isAtInstruction(t_script, R3_VIGILANTE_POLICE_TEST_OFFSET)
			|| isAtInstruction(t_script, R3_VIGILANTE_HUNTER_TEST_OFFSET);
	}
}

void SubmissionStartBlocked::install(const std::vector<std::unique_ptr<SubmissionTracker>>& t_trackers)
{
	g_trackers = &t_trackers;

	ScriptCommandHook::blockCommand(COMMAND_IS_CHAR_IN_ANY_POLICE_VEHICLE, &blockVigilanteGate);
	ScriptCommandHook::blockCommand(COMMAND_IS_CHAR_IN_MODEL, &blockVigilanteGate);
}
