#pragma once

class Lockable
{
public:
	virtual ~Lockable() = default;

	virtual bool isVehicleValid(int t_modelId) const { return false; }

	void unlock();
	void setGated(bool t_gated);
	bool isUnlocked() const;

protected:
	bool m_isUnlocked = false;
	bool m_isGated = false;
};
