#pragma once
#include "Collectible.h"

// The 50 Las Venturas horseshoes. Collected by touching them, so the one just picked up is simply
// the nearest unclaimed - no camera ray, unlike snapshots.
class HorseshoeTracker : public Collectible<50>
{
public:
	HorseshoeTracker();

	bool update() override;

protected:
	float readCount() const override;
	int identifyCollected() const override;
};
