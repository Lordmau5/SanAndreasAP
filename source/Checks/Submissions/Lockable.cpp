#include "Lockable.h"

void Lockable::unlockVehicles()
{
	m_vehiclesUnlocked = true;
}

void Lockable::setVehiclesGated(bool t_gated)
{
	m_vehiclesGated = t_gated;
}

bool Lockable::vehiclesUnlocked() const
{
	return !m_vehiclesGated || m_vehiclesUnlocked;
}
