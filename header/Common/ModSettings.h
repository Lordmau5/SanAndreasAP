#pragma once
#include <CRGBA.h>

namespace ModSettings
{
	enum class ItemColour
	{
		Progression,
		Useful,
		Trap,
		Filler,
		Count,
	};

	void load();
	float notificationSeconds();
	float collectibleNumberScale();
	float missionCounterScale();
	const CRGBA& itemColour(ItemColour t_which);
	int unselectedDimPercent();
	bool fastTravelEnabled();
}
