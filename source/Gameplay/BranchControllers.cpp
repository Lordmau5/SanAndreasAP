#include "BranchControllers.h"
#include "RunningScripts.h"
#include "ScriptGlobals.h"
#include "BranchProgress.h"
#include "CTheScripts.h"
#include <eScriptCommands.h>
#include <extensions/ScriptCommands.h>

namespace
{
	class BranchController
	{
	public:
		const char* scriptName;
		int address;
		int counterOffset;
		int finishedAt;
		int positionOffset;
		int spriteOffset;
		int blipHandleOffset;
		int blipDisplay;
		int requiresMission;
	};

	constexpr int NO_PREREQUISITE = 0;
	constexpr int LEAVE_BLIP_DISPLAY = -1;
	constexpr int BLIP_ONLY = 2;

	constexpr BranchController CONTROLLERS[] = {
		{ "SWEET", 62090, 1808, 9, 1848, 1768, 1740, LEAVE_BLIP_DISPLAY, NO_PREREQUISITE },
		{ "RYDER", 63143, 1812, 3, 1860, 1772, 1744, LEAVE_BLIP_DISPLAY, 12 },
		{ "SMOKE", 63492, 1816, 4, 1872, 1780, 1736, LEAVE_BLIP_DISPLAY, NO_PREREQUISITE },
		{ "STRAP", 63835, 1820, 5, 1884, 1784, 1748, LEAVE_BLIP_DISPLAY, NO_PREREQUISITE },
		{ "CRASH", 62904, 1824, 2, 1908, 1776, 1752, LEAVE_BLIP_DISPLAY, NO_PREREQUISITE },
		{ "CESAR", 64462, 1828, 1, 1920, 1788, 1760, BLIP_ONLY, NO_PREREQUISITE },
	};

	bool branchFinished(const BranchController& t_controller)
	{
		return ScriptGlobals::read(ScriptGlobals::slotOf(t_controller.counterOffset))
			>= t_controller.finishedAt;
	}

	void raiseMarkerBlip(const BranchController& t_controller)
	{
		if (ScriptGlobals::read(ScriptGlobals::slotOf(t_controller.blipHandleOffset)) != 0) return;

		int sprite = ScriptGlobals::read(ScriptGlobals::slotOf(t_controller.spriteOffset));
		if (sprite == 0) return;

		int slot = ScriptGlobals::slotOf(t_controller.positionOffset);
		int handle = 0;
		plugin::Command<eScriptCommands::COMMAND_ADD_SPRITE_BLIP_FOR_CONTACT_POINT>(
			ScriptGlobals::readFloat(slot),
			ScriptGlobals::readFloat(slot + 1),
			ScriptGlobals::readFloat(slot + 2),
			sprite, &handle);

		ScriptGlobals::write(ScriptGlobals::slotOf(t_controller.blipHandleOffset), handle);

		if (t_controller.blipDisplay != LEAVE_BLIP_DISPLAY)
		{
			plugin::Command<eScriptCommands::COMMAND_CHANGE_BLIP_DISPLAY>(handle, t_controller.blipDisplay);
		}
	}
}

void BranchControllers::update(const BranchProgress& t_progress)
{
	for (const BranchController& controller : CONTROLLERS)
	{
		if (controller.requiresMission != NO_PREREQUISITE
			&& !t_progress.missionCompleted(controller.requiresMission))
		{
			continue;
		}

		if (branchFinished(controller)) continue;

		if (!RunningScripts::isActive(controller.scriptName))
		{
			CTheScripts::StartNewScript(
				reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace) + controller.address);
		}

		raiseMarkerBlip(controller);
	}
}
