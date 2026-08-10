#pragma once
#include <vector>
#include "Position.h"

class CObject;
class CVector;
class BranchProgress;

class MissionBlockerManager
{
public:
	void update(const BranchProgress& t_progress);

	void removeAll();

	void forget();

private:
	void reconcile(const BranchProgress& t_progress);
	void spawnBlocker(const Position& t_pos, int t_modelId);
	void removeBlockersAt(const Position& t_pos);
	void adoptExisting();
	bool isBlockerPosition(const CVector& t_position, int t_modelId) const;
	bool ownsBlocker(const CObject* t_object) const;
	bool hasBlockerAt(const Position& t_spawn, int t_modelId) const;

	std::vector<CObject*> m_blockers;
	int m_scanTicks = 0;
};
