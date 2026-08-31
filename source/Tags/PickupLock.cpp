#include "PickupLock.h"
#include <CPickups.h>

void PickupLock::setModelLocked(int t_modelIndex, bool t_locked)
{
	bool wantVisible = !t_locked;

	for (unsigned int i = 0; i < MAX_NUM_PICKUPS; ++i)
	{
		CPickup& pickup = CPickups::aPickUps[i];
		if (pickup.m_nPickupType == PICKUP_NONE) continue;
		if (pickup.m_nModelIndex != t_modelIndex) continue;
		if ((pickup.m_nFlags.bVisible != 0) == wantVisible) continue;

		pickup.m_nFlags.bVisible = wantVisible;
		if (t_locked) pickup.GetRidOfObjects();
	}
}
