#pragma once
#include <array>
#include <CVector.h>
#include "PendingChecks.h"

class SaveDataManager;

class TagTracker
{
public:
	// Poll once. Returns true when a tag check is waiting to be sent.
	bool update();

	// Re-takes the "tags sprayed last tick" baseline. Detection diffs against it, and a freshly
	// loaded save has a different count than the running session - without this that jump reads as
	// fresh progress and fires phantom checks.
	void resyncBaseline();

	bool hasPending() const { return m_pending.hasPending(); }
	int getPendingIndex() const;
	void confirmSent() { m_pending.confirm(); }

	const std::array<bool, 100>& getClaimed() const { return m_claimed; }

	void save(SaveDataManager& t_saveData) const;
	void load(const SaveDataManager& t_saveData);

private:
	// Claims the t_count nearest unsprayed tags to the player and queues each as a check.
	void claimNearest(const CVector& t_playerPos, int t_count);

	std::array<bool, 100> m_claimed{};
	PendingChecks<int> m_pending;

	float m_lastCount = 0.0f;
	bool m_countInitialized = false;
};
