#pragma once
#include <string>
#include "NotificationOverlay.h"

// What an incoming GIVE: effect actually does. Mod switches on this, so adding an item means
// adding an enumerator and a table row - and the compiler can point at a switch that hasn't
// handled the new one yet.
enum class ItemEffect
{
	Money,
	Weapon,
	ProgressiveMission,
	ProgressiveMap,
	SubmissionCheck, // unlocks one submission's checks; which one is in submissionId
	MaxSkill,
	WeaponMastery,   // maxes one weapon's skill; which one arrives as the value
	ArmorRefill,
	CarRepair,
	StreetRaces,
	Trap,
};

class ItemEffectSpec
{
public:
	const char* name;
	ItemEffect effect;
	int submissionId;
	const char* trapName;
	const char* message;
	NotificationIcon icon;
};

const ItemEffectSpec* findItemEffect(const std::string& t_name);

std::string formatItemMessage(const ItemEffectSpec& t_spec, const std::string& t_value);
