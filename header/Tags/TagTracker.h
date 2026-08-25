 #pragma once
#include "Collectible.h"

class TagTracker : public Collectible<100>
{
public:
	TagTracker();

	bool update() override;
	const char* consumeLockedNotice() override;

private:
	bool m_noticePending = false;

protected:
	float readCount() const override;
	int identifyCollected() const override;
};
