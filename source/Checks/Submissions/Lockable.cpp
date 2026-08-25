#include "Lockable.h"

void Lockable::unlockVehicles()
{
	m_vehiclesUnlocked = true;
}

bool Lockable::vehiclesUnlocked() const
{
	return m_vehiclesUnlocked;
}
