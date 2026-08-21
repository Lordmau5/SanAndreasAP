#include "StreetRaceUnlock.h"
#include "EntityIDs.h"
#include "ScriptGlobals.h"
#include <CRunningScript.h>
#include <cstring>

namespace
{
	constexpr int SCRIPT_BASE_OFFSET = 183113;
	constexpr int MAX_LINES = 64;

	constexpr int SCHOOL_BODY_OFFSET = 757;
	constexpr int SCHOOL_BODY_END = 867;

	constexpr int RACE_BODY_OFFSET = 988;
	constexpr int RACE_BODY_END = 1079;

	void runScript(CRunningScript& t_script, unsigned char* t_from, unsigned char* t_to)
	{
		t_script.m_pCurrentIP = t_from;
		for (int scriptLine = 0; scriptLine < MAX_LINES && t_script.m_pCurrentIP != t_to; ++scriptLine)
		{
			t_script.ProcessOneCommand();
		}
	}
}

void StreetRaceUnlock::update()
{
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
