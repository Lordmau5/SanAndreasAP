#pragma once

class Lockable
{
public:
	virtual ~Lockable() = default;

	virtual bool locksVehicleModel(int t_modelId) const { return false; }

	void unlockVehicles();
	void setVehiclesGated(bool t_gated);
	bool vehiclesUnlocked() const;

protected:
	bool m_vehiclesUnlocked = false;
	bool m_vehiclesGated = false;
};
