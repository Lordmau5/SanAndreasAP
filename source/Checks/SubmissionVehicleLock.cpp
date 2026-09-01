#include "SubmissionVehicleLock.h"
#include "Submissions/SubmissionTracker.h"
#include <CPools.h>
#include <CVehicle.h>

namespace
{
	bool anyTrackerLocks(const std::vector<std::unique_ptr<SubmissionTracker>>& t_trackers, int t_modelId)
	{
		for (const auto& tracker : t_trackers)
		{
			if (!tracker->locksVehicleModel(t_modelId)) continue;
			if (!tracker->vehiclesUnlocked()) return true;
		}
		return false;
	}
}

void SubmissionVehicleLock::update(const std::vector<std::unique_ptr<SubmissionTracker>>& t_trackers)
{
	auto* pool = CPools::ms_pVehiclePool;
	if (!pool) return;

	// Disable in favor of new submission start blocking code
	if (true) return;

	for (int i = 0; i < pool->m_nSize; ++i)
	{
		CVehicle* vehicle = pool->GetAt(i);
		if (!vehicle) continue;

		if (vehicle->m_nCreatedBy == MISSION_VEHICLE) continue;

		if (anyTrackerLocks(t_trackers, vehicle->m_nModelIndex))
		{
			vehicle->m_eDoorLock = DOORLOCK_LOCKOUT_PLAYER_ONLY;
		}
		else if (vehicle->m_eDoorLock == DOORLOCK_LOCKOUT_PLAYER_ONLY)
		{
			vehicle->m_eDoorLock = DOORLOCK_UNLOCKED;
		}
	}
}
