#pragma once
#include "Collectible.h"

// The 100 Los Santos spray tags. Spraying requires standing at the wall, so the tag the player is
// nearest to is the one they just sprayed. See Collectible for everything shared.
class TagTracker : public Collectible<100>
{
public:
	TagTracker();

	// Highlight one tag (0-based, -1 clears), driven by the client's /tag command. The highlighted
	// tag is always blipped and full-range, so it can be followed from anywhere.
	void setLocated(int t_index);

protected:
	float readCount() const override;
	int identifyCollected() const override;
	int locatedIndex() const override { return m_located; }

private:
	int m_located = -1;
};
