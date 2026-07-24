#include "TagTracker.h"
#include "TagPositions.h"
#include "SaveDataManager.h"
#include "common.h"
#include <algorithm>
#include <vector>

namespace
{
	constexpr char TAGS_CLAIMED_KEY[] = "tags_claimed";
}

bool TagTracker::update()
{
	float currentCount = static_cast<float>(*reinterpret_cast<int32_t*>(TAGS_SPRAYED_ADDR));

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

		if (CPlayerPed* player = FindPlayerPed())
		{
			claimNearest(player->GetPosition(), delta);
		}
	}

	return hasPending();
}

void TagTracker::resyncBaseline()
{
	m_lastCount = static_cast<float>(*reinterpret_cast<int32_t*>(TAGS_SPRAYED_ADDR));
}

int TagTracker::getPendingIndex() const
{
	if (!m_pending.hasPending()) return -1;
	return m_pending.front();
}

void TagTracker::claimNearest(const CVector& t_playerPos, int t_count)
{
	std::vector<std::pair<float, int>> distances;
	for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
	{
		if (m_claimed[i]) continue;
		distances.push_back({ CVector::Distance(t_playerPos, tagPositions[i]), i });
	}
	std::sort(distances.begin(), distances.end(),
		[](const auto& a, const auto& b) { return a.first < b.first; });

	for (int i = 0; i < t_count && i < static_cast<int>(distances.size()); ++i)
	{
		int tagIndex = distances[i].second;
		m_claimed[tagIndex] = true;
		m_pending.push(tagIndex);
	}
}

void TagTracker::save(SaveDataManager& t_saveData) const
{
	std::string bits(m_claimed.size(), '0');
	for (size_t i = 0; i < m_claimed.size(); ++i)
	{
		if (m_claimed[i]) bits[i] = '1';
	}
	t_saveData.setValue(TAGS_CLAIMED_KEY, bits);
}

void TagTracker::load(const SaveDataManager& t_saveData)
{
	// A save written before this key existed (or a shorter string from an older build) leaves the
	// remaining tags unclaimed rather than reading past the end.
	std::string bits = t_saveData.getValue(TAGS_CLAIMED_KEY, std::string(100, '0'));
	for (size_t i = 0; i < m_claimed.size(); ++i)
	{
		m_claimed[i] = i < bits.size() && bits[i] == '1';
	}
}
