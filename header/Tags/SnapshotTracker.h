#pragma once
#include "Collectible.h"

class SnapshotTracker : public Collectible<50>
{
public:
	SnapshotTracker();
	bool update() override;

protected:
	float readCount() const override;
	int identifyCollected() const override;

private:
	static constexpr float MIN_AIM_DOT = 0.5f;
	static constexpr float AIM_TIE_EPSILON = 0.02f;
};
