#pragma once
#include <plugin.h>
#include <string>
#include "CWorld.h"

class CheckGiver
{
public:
	CheckGiver() = default;

	void giveMoney(int t_amount);
	void giveWeapon(const std::string& t_weaponType, bool t_equip = false);
	void giveProgressiveMap();

	void giveArmorRefill();

	void giveWeaponMastery(const std::string& t_weaponName);

	void giveCarRepair();

	void update();
private:
	bool m_carRepairPending = false;
};
