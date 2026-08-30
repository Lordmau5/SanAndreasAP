#pragma once

class CRunningScript;

namespace ScriptCommandHook
{
	using CommandOverride = bool(*)(CRunningScript*);
	void blockCommand(unsigned short t_commandId, CommandOverride t_commandOverride);

	using CommandReplacement = bool(*)(CRunningScript*);
	void replaceCommand(unsigned short t_commandId, CommandReplacement t_commandReplacement);
}
