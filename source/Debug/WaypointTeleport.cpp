#include "WaypointTeleport.h"

#ifdef DEBUG

#include "EdgeTriggeredKey.h"
#include "PlayerControl.h"
#include "common.h"
#include "CStreaming.h"
#include <CRadar.h>
#include <CGame.h>
#include <CWorld.h>
#include <CVehicle.h>

namespace
{
	EdgeTriggeredKey g_teleportKey{ VK_F6 };

	constexpr float DROP_HEIGHT = 1.0f;

	const tRadarTrace* findWaypoint()
	{
		for (unsigned int t = 0; t < MAX_RADAR_TRACES; ++t)
		{
			const tRadarTrace& trace = CRadar::ms_RadarTrace[t];
			if (trace.m_bInUse && trace.m_nRadarSprite == RADAR_SPRITE_WAYPOINT) return &trace;
		}
		return nullptr;
	}
}

void WaypointTeleport::update()
{
	if (!g_teleportKey.justPressed() || !PlayerControl::isInControl()) return;

	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	const tRadarTrace* waypoint = findWaypoint();
	if (!waypoint) return;

	CVector destination(waypoint->m_vecPos.x, waypoint->m_vecPos.y, 0.0f);
	CStreaming::LoadScene(&destination);
	destination.z = CWorld::FindGroundZForCoord(destination.x, destination.y) + DROP_HEIGHT;

	CVehicle* vehicle = player->m_pVehicle;
	if (vehicle && player->bInVehicle)
	{
		vehicle->Teleport(destination, false);
	}
	else
	{
		player->Teleport(destination, false);
	}

	player->m_nAreaCode = 0;
	CGame::currArea = 0;
}

#else

void WaypointTeleport::update()
{
}

#endif
