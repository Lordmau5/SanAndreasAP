#include "Lockable.h"

void Lockable::unlock()
{
	m_isUnlocked = true;
}

void Lockable::setGated(bool t_gated)
{
	m_isGated = t_gated;
}

bool Lockable::isUnlocked() const
{
	return !m_isGated || m_isUnlocked;
}
