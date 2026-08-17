#include "MissionLocateBlocked.h"
#include "BranchProgress.h"
#include "MissionBranches.h"
#include <CRunningScript.h>
#include <CTheScripts.h>
#include <eScriptCommands.h>
#include <Patch.h>

namespace
{
	constexpr size_t LOCATE_COMMAND_BLOCK = COMMAND_LOCATE_CHAR_ANY_MEANS_3D / 100;
	constexpr short LOCATE_PARAM_COUNT = 8;
	using CommandHandler = unsigned char(__thiscall*)(CRunningScript*, unsigned short);
	CommandHandler g_original = nullptr;
	const BranchProgress* g_progress = nullptr;

	bool isLocateChar3D(unsigned short t_commandId)
	{
		return t_commandId == COMMAND_LOCATE_CHAR_ANY_MEANS_3D
			|| t_commandId == COMMAND_LOCATE_CHAR_ON_FOOT_3D
			|| t_commandId == COMMAND_LOCATE_CHAR_IN_CAR_3D;
	}

	float collectedFloat(int t_index)
	{
		return *reinterpret_cast<float*>(&ScriptParams[t_index]);
	}

	unsigned char __fastcall onLocateCommandBlock(CRunningScript* t_script, void*, unsigned short t_commandId)
	{
		if (!isLocateChar3D(t_commandId) || t_script->m_bIsMission || !g_progress)
		{
			return g_original(t_script, t_commandId);
		}

		unsigned char* resumeIP = t_script->m_pCurrentIP;
		t_script->CollectParameters(LOCATE_PARAM_COUNT);
		int marker = missionMarkerIndexAt(collectedFloat(1), collectedFloat(2));
		bool blocked = false;
		if (marker >= 0 && markerIsBlocked(marker, *g_progress))
		{
			blocked = true;
		}
		t_script->m_pCurrentIP = resumeIP;

		unsigned char result = g_original(t_script, t_commandId);
		if (blocked) t_script->m_bCondResult = false;
		return result;
	}
}

void MissionLocateBlocked::install(const BranchProgress& t_progress)
{
	if (g_original) return;

	g_progress = &t_progress;
	g_original = CRunningScript::CommandHandlerTable[LOCATE_COMMAND_BLOCK];

	plugin::patch::SetPointer(
		reinterpret_cast<uintptr_t>(&CRunningScript::CommandHandlerTable[LOCATE_COMMAND_BLOCK]),
		reinterpret_cast<void*>(&onLocateCommandBlock));
}
