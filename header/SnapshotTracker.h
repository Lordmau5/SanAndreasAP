#pragma once
#include "Collectible.h"

// The 50 San Fierro snapshots. Photos are taken at range and often zoomed, so the subject is NOT
// the entry the player stands nearest to - it picks the unclaimed snapshot the camera is aimed at.
// See Collectible for everything shared.
class SnapshotTracker : public Collectible<50>
{
public:
	SnapshotTracker();

	// TEMPORARY: which entry the camera ray would pick right now, for the debug overlay. Points at a
	// snapshot and this names it before the shutter is even pressed.
	int getAimedIndex() const { return identifyCollected(); }

protected:
	float readCount() const override;
	int identifyCollected() const override;

private:
	// How far off-centre a snapshot may sit and still count as the subject. 0.5 is a 60-degree
	// half-angle: wider than any zoom level, so a legitimate capture is never missed, while an entry
	// off to the side or behind the player still scores too low to win.
	static constexpr float MIN_AIM_DOT = 0.5f;
	// Two targets this close in angle are effectively in line, so the nearer one is the subject.
	static constexpr float AIM_TIE_EPSILON = 0.02f;
};
