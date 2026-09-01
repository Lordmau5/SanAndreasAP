#include "SubmissionStartBlocked.h"
#include "Submissions/SubmissionTracker.h"
#include "ScriptCommandHook.h"
#include "EntityIDs.h"
#include <CRunningScript.h>
#include <CTheScripts.h>
#include <eScriptCommands.h>
#include "common.h"
#include <CHud.h>

namespace
{
	const std::vector<std::unique_ptr<SubmissionTracker>>* g_trackers = nullptr;

	bool preventSubmissionStart(CRunningScript* t_script, int submissionID = -1, int vehicleID = -1)
	{
		if (_strnicmp(t_script->m_szName, "R3", 10) != 0) return false;
		if (!g_trackers) return false;

		for (const auto& tracker : *g_trackers)
		{
			if (submissionID != -1)
			{
				if (tracker->getSubmissionID() != submissionID) continue;
			}
			else if (vehicleID != -1)
			{
				auto vehicles = tracker->getSubmissionValidVehicles();
				if (std::find(vehicles.begin(), vehicles.end(), ScriptParams[1]) == vehicles.end()) continue;
			}

			t_script->UpdateCompareFlag(tracker->vehiclesUnlocked());
			return true;
		}

		return false;
	}

	bool preventSubmissionStart_Generic(CRunningScript* t_script)
	{
		t_script->CollectParameters(2);

		return preventSubmissionStart(t_script, -1, ScriptParams[1]);
	}

	bool preventSubmissionStart_Taxi(CRunningScript* t_script)
	{
		return preventSubmissionStart(t_script, TAXI_ID, -1);
	}

	bool preventSubmissionStart_Police(CRunningScript* t_script)
	{
		return preventSubmissionStart(t_script, VIGILANTE_ID, -1);
	}
}

void SubmissionStartBlocked::update(const std::vector<std::unique_ptr<SubmissionTracker>>& t_trackers)
{
	g_trackers = &t_trackers;

	ScriptCommandHook::blockCommand(COMMAND_IS_CHAR_IN_MODEL, &preventSubmissionStart_Generic);
	ScriptCommandHook::blockCommand(COMMAND_IS_CHAR_IN_TAXI, &preventSubmissionStart_Taxi);
	ScriptCommandHook::blockCommand(COMMAND_IS_CHAR_IN_ANY_POLICE_VEHICLE, &preventSubmissionStart_Police);

	keyHandler();
}

void SubmissionStartBlocked::keyHandler()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;
	if (!player->bInVehicle || !player->m_pVehicle) return;

	auto pad = CPad::GetPad(0);
	if (!pad || !pad->GetLookBehindForPed()) return;

	if (CHud::HelpMessageDisplayed()) return;

	int playerVehicleModel = player->m_pVehicle->m_nModelIndex;

	for (const auto& tracker : *g_trackers)
	{
		auto vehicles = tracker->getSubmissionValidVehicles();
		if (std::find(vehicles.begin(), vehicles.end(), playerVehicleModel) == vehicles.end()) continue;

		// Submission unlocked
		if (tracker->vehiclesUnlocked()) return;

		CHud::SetHelpMessage("You have not unlocked this submission yet.", true, false, false);
		CHud::m_nHelpMessageTimer = 5; // Force set it to 5 seconds in case we show it while another help message is already shown
	}
}