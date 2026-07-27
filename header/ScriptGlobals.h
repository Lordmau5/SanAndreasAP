#pragma once
#include "CTheScripts.h"

// The SCM's global variable array - where state with no CStats entry lives (driving school scores).
namespace ScriptGlobals
{
	// One 4-byte slot each, at the start of ScriptSpace.
	inline constexpr int COUNT = 16384;

	inline int read(int t_index)
	{
		if (t_index < 0 || t_index >= COUNT) return 0;
		return *reinterpret_cast<int*>(CTheScripts::ScriptSpace + t_index * 4);
	}
}
