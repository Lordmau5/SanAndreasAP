#pragma once
#include "Collectible.h"

// The 30 vehicles on the Easter Basin export board. The game only counts how many have been
// shipped, so which one it was is recovered from the last board vehicle CJ drove - by the time the
// counter ticks he has usually already left it at the crane.
class ExportListTracker : public Collectible<30>
{
public:
	ExportListTracker();

	// Records the driven vehicle before the base class diffs the counter.
	bool update() override;

protected:
	float readCount() const override;
	int identifyCollected() const override;

private:
	void rememberDrivenVehicle();

	int m_lastDrivenIndex = -1;
};
