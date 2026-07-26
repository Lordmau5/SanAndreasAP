#pragma once

// CJ has one fighting-style slot, but several gyms can each grant a style. When more than one has
// been received, the most-recently-received wins, so a freshly granted style overwrites an earlier
// one. Every gym shares one arbiter: each receipt claims a new order, and the highest order claimed
// is the current winner.
class FightingStyleArbiter
{
public:
	// Returns a fresh order and makes it the latest, so the caller becomes the winner.
	int claim() { return ++m_latest; }

	// The order of the current winner. A gym owns the style iff its claimed order equals this.
	int latest() const { return m_latest; }

private:
	int m_latest = 0;
};
