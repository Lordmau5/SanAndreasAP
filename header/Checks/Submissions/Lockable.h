#pragma once

class Lockable
{
public:
	virtual ~Lockable() = default;

	virtual bool locksVehicleModel(int t_modelId) const { return false; }

	void unlockVehicles();
	bool vehiclesUnlocked() const;

protected:
	bool m_vehiclesUnlocked = false;
};
