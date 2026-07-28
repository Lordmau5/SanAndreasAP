#pragma once
#include <array>
#include <span>
#include <string>
#include <vector>
#include "PendingChecks.h"
#include "SaveDataManager.h"
#include "APProtocol.h"
#include "BlipTarget.h"

// Non-templated interface over every collectible tracker, so CheckListener can hold them in one
// list and drive them alike - poll them, persist them, drain their checks, describe their blips -
// with no case per kind. getClaimed() is deliberately NOT here: its return type is size-dependent,
// and it is a tag-blip-drawing concern rather than a shared one.
class CollectibleTracker
{
public:
	virtual ~CollectibleTracker() = default;

	virtual bool update() = 0;
	virtual void resyncBaseline() = 0;
	virtual void save(SaveDataManager& t_saveData) const = 0;
	virtual void load(const SaveDataManager& t_saveData) = 0;

	virtual bool hasPending() const = 0;
	virtual int getPendingIndex() const = 0;
	virtual void confirmSent() = 0;
	// The wire line for one collected entry, e.g. "CHECK:TAG:41\n".
	virtual std::string buildCheckMessage(int t_index) const = 0;

	// Appends one BlipTarget per entry (claimed included, so the manager can reap old blips). The
	// caller ranks them and hands the whole lot to CollectibleBlipsManager.
	virtual void appendBlipTargets(std::vector<BlipTarget>& t_out) const = 0;
};

// Base for the scattered-collectible trackers (spray tags, snapshots). The game exposes only a
// running COUNT of how many have been collected, never which one, so each tick diffs that count and
// asks the subclass which entry was just collected. Claimed entries are persisted as a bitstring
// and queued as checks.
//
// Templated on the entry count so getClaimed() keeps returning a plain std::array<bool, N> - the
// blip code depends on that. The non-templated CollectibleTracker above is what lets the two be
// stored and driven together despite the differing N.
//
// Subclasses supply only what differs: where the count comes from (readCount) and how to tell which
// entry was just collected (identifyCollected).
template <int N>
class Collectible : public CollectibleTracker
{
public:

	// Poll once. Returns true when a check is waiting to be sent.
	bool update() override
	{
		float currentCount = readCount();

		// The first read only establishes the baseline - the counter already holds whatever the
		// loaded save had, so treating it as progress would fire phantom checks.
		if (!m_countInitialized)
		{
			m_lastCount = currentCount;
			m_countInitialized = true;
			return hasPending();
		}

		int delta = static_cast<int>(currentCount) - static_cast<int>(m_lastCount);
		if (delta > 0)
		{
			m_lastCount = currentCount;

			// Claiming the nearest/aimed one, then the next, delta times gives the delta collected
			// this tick, in the order they'd be found - identical to one batched search.
			for (int i = 0; i < delta; ++i)
			{
				int index = identifyCollected();
				// Can't tell which - drop the check rather than claim the wrong entry, which would
				// burn a location the player never found.
				if (index < 0) break;

				m_claimed[index] = true;
				m_pending.push(index);
			}
		}

		return hasPending();
	}

	// Re-takes the "collected count last tick" baseline. Detection diffs against it, and a freshly
	// loaded save has a different count than the running session - without this that jump reads as
	// fresh progress and fires phantom checks.
	void resyncBaseline() override { m_lastCount = readCount(); }

	bool hasPending() const override { return m_pending.hasPending(); }
	int getPendingIndex() const override { return m_pending.hasPending() ? m_pending.front() : -1; }
	void confirmSent() override { m_pending.confirm(); }

	std::string buildCheckMessage(int t_index) const override { return APProtocol::collectibleCheck(m_checkType, t_index); }

	void appendBlipTargets(std::vector<BlipTarget>& t_out) const override
	{
		// Not everything counted this way sits somewhere on the map - the export vehicles move, so
		// that set is constructed with no positions and has nothing to blip.
		if (m_positions.empty()) return;

		for (int i = 0; i < N; ++i)
		{
			t_out.push_back({ m_positions[i], m_sprite, i + 1, m_claimed[i], i == locatedIndex(), INT_MAX });
		}
	}

	const std::array<bool, N>& getClaimed() const { return m_claimed; }

	void save(SaveDataManager& t_saveData) const override
	{
		std::string bits(N, '0');
		for (int i = 0; i < N; ++i)
		{
			if (m_claimed[i]) bits[i] = '1';
		}
		t_saveData.setValue(m_saveKey, bits);
	}

	void load(const SaveDataManager& t_saveData) override
	{
		// A save written before this key existed (or a shorter string from an older build) leaves
		// the remaining entries unclaimed rather than reading past the end.
		std::string bits = t_saveData.getValue(m_saveKey, std::string(N, '0'));
		for (int i = 0; i < N; ++i)
		{
			m_claimed[i] = i < static_cast<int>(bits.size()) && bits[i] == '1';
		}
	}

protected:
	Collectible(std::span<const CVector> t_positions, int t_sprite,
		const char* t_saveKey, const char* t_checkType)
		: m_positions(t_positions), m_sprite(t_sprite), m_saveKey(t_saveKey), m_checkType(t_checkType) {}

	// How many of this collectible the game says have been collected so far.
	virtual float readCount() const = 0;

	// The next unclaimed entry that was just collected, or -1 if it can't be determined. Called
	// once per increment of the count, so it may assume anything it returned earlier is claimed.
	virtual int identifyCollected() const = 0;

	// The entry the player asked to have highlighted (the /tag command), or -1. Only tags override
	// this; everything else has no locate command.
	virtual int locatedIndex() const { return -1; }

	bool isClaimed(int t_index) const { return m_claimed[t_index]; }

private:
	std::span<const CVector> m_positions;
	int m_sprite;
	const char* m_saveKey;
	const char* m_checkType;
	std::array<bool, N> m_claimed{};
	PendingChecks<int> m_pending;
	float m_lastCount = 0.0f;
	bool m_countInitialized = false;
};
