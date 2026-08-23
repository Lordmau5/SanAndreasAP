#include "StreetRaceUnlock.h"
#include "EntityIDs.h"
#include "ScriptGlobals.h"
#include <CRunningScript.h>
#include <eScriptCommands.h>
#include <Patch.h>
#include <cstring>

namespace
{
	constexpr int SCRIPT_BASE_OFFSET = 183113;

	constexpr int SCHOOL_BODY_OFFSET = 757;
	constexpr int SCHOOL_BODY_END = 867;

	constexpr int RACE_BODY_OFFSET = 988;
	constexpr int RACE_BODY_END = 1079;

	void runScript(CRunningScript& t_script, unsigned char* t_from, unsigned char* t_to)
	{
		t_script.m_pCurrentIP = t_from;
		while (t_script.m_pCurrentIP < t_to)
		{
			t_script.ProcessOneCommand();
		}
	}

	constexpr size_t COMPARE_COMMAND_BLOCK = COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER / 100;
	constexpr short COMPARE_PARAM_COUNT = 2;
	constexpr int DESERT_MISSIONS_REQUIRED = 3;

	using CommandHandler = unsigned char(__thiscall*)(CRunningScript*, unsigned short);
	CommandHandler g_original = nullptr;
	bool g_blockVanillaUnlock = false;

	unsigned char __fastcall onCompareCommandBlock(CRunningScript* t_script, void*, unsigned short t_commandId)
	{
		bool isGate = false;

		if (g_blockVanillaUnlock
			&& t_commandId == COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER
			&& _strnicmp(t_script->m_szName, "MOB_SF", 8) == 0)
		{
			unsigned char* resumeIP = t_script->m_pCurrentIP;
			t_script->CollectParameters(COMPARE_PARAM_COUNT);
			isGate = ScriptParams[1] == DESERT_MISSIONS_REQUIRED;
			t_script->m_pCurrentIP = resumeIP;
		}

		unsigned char result = g_original(t_script, t_commandId);

		if (isGate) t_script->m_bCondResult = false;
		return result;
	}
}

void StreetRaceUnlock::blockVanillaUnlock()
{
	g_blockVanillaUnlock = true;
	if (g_original) return;

	g_original = CRunningScript::CommandHandlerTable[COMPARE_COMMAND_BLOCK];

	plugin::patch::SetPointer(
		reinterpret_cast<uintptr_t>(&CRunningScript::CommandHandlerTable[COMPARE_COMMAND_BLOCK]),
		reinterpret_cast<void*>(&onCompareCommandBlock));
}

void StreetRaceUnlock::update(bool t_itemReceived)
{
	if (!t_itemReceived) return;
	if (ScriptGlobals::read(STREET_RACES_UNLOCKED_GLOBAL) != 0) return;
	if (ScriptGlobals::read(TRACE_MARKER_X_GLOBAL) == 0) return;

	unsigned char* scriptBase = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace) + SCRIPT_BASE_OFFSET;

	int savedParams[32];
	memcpy(savedParams, ScriptParams, sizeof(savedParams));

	CRunningScript script;
	script.Init();
	script.m_pBaseIP = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace);

	if (ScriptGlobals::read(DRIVING_SCHOOL_UNLOCKED_GLOBAL) == 0)
	{
		runScript(script, scriptBase + SCHOOL_BODY_OFFSET, scriptBase + SCHOOL_BODY_END);
	}
	runScript(script, scriptBase + RACE_BODY_OFFSET, scriptBase + RACE_BODY_END);

	memcpy(ScriptParams, savedParams, sizeof(savedParams));
}
