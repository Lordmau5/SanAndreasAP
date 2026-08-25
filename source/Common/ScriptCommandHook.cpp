#include "ScriptCommandHook.h"
#include <CRunningScript.h>
#include <Patch.h>
#include <vector>

namespace
{
	constexpr size_t COMMANDS_PER_HANDLER = 100;
	constexpr size_t HANDLER_COUNT = 27;

	using CommandHandler = unsigned char(__thiscall*)(CRunningScript*, unsigned short);

	class CommandPair
	{
	public:
		unsigned short commandId;
		ScriptCommandHook::CommandOverride commandOverride;
	};

	CommandHandler g_originalHandlers[HANDLER_COUNT] = {};
	std::vector<CommandPair> g_commandPairs;

	unsigned char __fastcall onCommand(CRunningScript* t_script, void*, unsigned short t_commandId)
	{
		bool isBlocked = false;

		for (const CommandPair& pair : g_commandPairs)
		{
			if (pair.commandId != t_commandId) continue;

			unsigned char* resumeIP = t_script->m_pCurrentIP;
			isBlocked = pair.commandOverride(t_script);
			t_script->m_pCurrentIP = resumeIP;

			if (isBlocked) break;
		}

		unsigned char result = g_originalHandlers[t_commandId / COMMANDS_PER_HANDLER](t_script, t_commandId);

		if (isBlocked) t_script->m_bCondResult = false;
		return result;
	}
}

void ScriptCommandHook::blockCommand(unsigned short t_commandId, CommandOverride t_commandOverride)
{
	for (const CommandPair& pair : g_commandPairs)
	{
		if (pair.commandId == t_commandId && pair.commandOverride == t_commandOverride) return;
	}
	g_commandPairs.push_back({ t_commandId, t_commandOverride });

	size_t handler = t_commandId / COMMANDS_PER_HANDLER;
	if (handler >= HANDLER_COUNT || g_originalHandlers[handler]) return;

	g_originalHandlers[handler] = CRunningScript::CommandHandlerTable[handler];

	plugin::patch::SetPointer(
		reinterpret_cast<uintptr_t>(&CRunningScript::CommandHandlerTable[handler]),
		reinterpret_cast<void*>(&onCommand));
}
