#pragma once
#include "CTheScripts.h"

// The SCM's global variable array. Anything the game tracks only in mission script state (school
// medals, for one) lives here rather than in CStats.
namespace ScriptGlobals
{
	// main.scm reserves the first chunk of ScriptSpace for globals, one 4-byte slot each.
	inline constexpr int COUNT = 16384;
	// Stands in for a global index that has not been confirmed against a running game yet.
	inline constexpr int UNVERIFIED_GLOBAL = -1;

	inline int read(int t_index)
	{
		if (t_index < 0 || t_index >= COUNT) return 0;
		return *reinterpret_cast<int*>(CTheScripts::ScriptSpace + t_index * 4);
	}
}
