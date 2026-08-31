#pragma once
#include "Collectible.h"

// The 50 statewide oysters.
class OysterTracker : public Collectible<50>
{
public:
	OysterTracker();

	bool update() override;

protected:
	float readCount() const override;
	int identifyCollected() const override;
};
