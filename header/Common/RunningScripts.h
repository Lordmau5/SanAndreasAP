#pragma once
#include "CTheScripts.h"
#include <cstring>

namespace RunningScripts
{
	inline bool isActive(const char* t_scriptName)
	{
		for (CRunningScript* script = CTheScripts::pActiveScripts; script; script = script->m_pNext)
		{
			if (_strnicmp(script->m_szName, t_scriptName, 8) == 0) return true;
		}
		return false;
	}
}
