#include "SubmissionStartBlocked.h"
#include "Submissions/SubmissionTracker.h"
#include "EntityIDs.h"
#include <CRunningScript.h>
#include <eScriptCommands.h>
#include <Patch.h>
#include <cstring>

namespace
{
	constexpr size_t POLICE_COMMAND_BLOCK = COMMAND_IS_CHAR_IN_ANY_POLICE_VEHICLE / 100;

	using CommandHandler = unsigned char(__thiscall*)(CRunningScript*, unsigned short);
	CommandHandler g_original = nullptr;
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

	bool isR3(CRunningScript* t_script)
	{
		return _strnicmp(t_script->m_szName, "R3", 8) == 0;
	}

	unsigned char __fastcall onPoliceCommandBlock(CRunningScript* t_script, void*, unsigned short t_commandId)
	{
		bool blocked = t_commandId == COMMAND_IS_CHAR_IN_ANY_POLICE_VEHICLE
			&& isR3(t_script)
			&& vigilanteLocked();

		unsigned char result = g_original(t_script, t_commandId);

		if (blocked) t_script->m_bCondResult = false;
		return result;
	}
}

void SubmissionStartBlocked::install(const std::vector<std::unique_ptr<SubmissionTracker>>& t_trackers)
{
	g_trackers = &t_trackers;
	if (g_original) return;

	g_original = CRunningScript::CommandHandlerTable[POLICE_COMMAND_BLOCK];

	plugin::patch::SetPointer(
		reinterpret_cast<uintptr_t>(&CRunningScript::CommandHandlerTable[POLICE_COMMAND_BLOCK]),
		reinterpret_cast<void*>(&onPoliceCommandBlock));
}
