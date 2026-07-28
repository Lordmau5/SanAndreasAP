#pragma once
#include "Collectible.h"

// The 100 Los Santos spray tags. Spraying requires standing at the wall, so the tag the player is
// nearest to is the one they just sprayed. See Collectible for everything shared.
class TagTracker : public Collectible<100>
{
public:
	TagTracker();

protected:
	float readCount() const override;
	int identifyCollected() const override;
};
