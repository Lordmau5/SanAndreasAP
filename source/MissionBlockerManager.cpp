#include "MissionBlockerManager.h"
#include "BranchProgress.h"
#include "MissionBranches.h"
#include "EntityIDs.h"
#include "common.h"
#include "CPools.h"
#include "CStreaming.h"
#include <CObject.h>
#include <CWorld.h>
#include <eModelID.h>

namespace
{
	constexpr int SCAN_INTERVAL = 30;
	constexpr int BLOCKER_MODEL_ID = 2973;
	constexpr int BARRICADE_MODEL_ID = MODEL_CJ_ROADBARRIER;
	constexpr float BARRICADE_Z_OFFSET = 0.6f;
	constexpr float BLOCKER_POSITION_TOLERANCE_SQ = 0.0001f;
}

void MissionBlockerManager::update(const BranchProgress& t_progress)
{
	size_t liveCount = 0;
	for (CObject* blocker : m_blockers)
	{
		if (CPools::ms_pObjectPool->IsObjectValid(blocker))
		{
			m_blockers[liveCount++] = blocker;
		}
	}
	m_blockers.resize(liveCount);

	if (++m_scanTicks < SCAN_INTERVAL || !FindPlayerPed()) return;
	m_scanTicks = 0;

	adoptExisting();

	if (m_blockers.size() > missionStartPos.size() * 2)
	{
		removeAll();
	}

	reconcile(t_progress);
}

void MissionBlockerManager::reconcile(const BranchProgress& t_progress)
{
	bool modelsRequested = false;

	size_t count = missionStartPos.size();
	if (count > MISSION_START_POS_BRANCH_COUNT) count = MISSION_START_POS_BRANCH_COUNT;

	for (size_t i = 0; i < count; ++i)
	{
		const char* branch = activeBranchAtMarker(i, t_progress);
		const Position& pos = missionStartPos[i];
		bool shouldBlock = branch != nullptr && t_progress.isBlocked(branch);

		bool hasBlocker = hasBlockerAt(pos, BLOCKER_MODEL_ID);
		bool hasBarricade = hasBlockerAt(pos, BARRICADE_MODEL_ID);

		if (shouldBlock)
		{
			if (hasBlocker && hasBarricade) continue;

			if (!modelsRequested)
			{
				CStreaming::RequestModel(BLOCKER_MODEL_ID, 0);
				CStreaming::RequestModel(BARRICADE_MODEL_ID, 0);
				CStreaming::LoadAllRequestedModels(false);
				modelsRequested = true;
			}
			if (!hasBlocker) spawnBlocker(pos, BLOCKER_MODEL_ID);
			if (!hasBarricade) spawnBlocker(pos, BARRICADE_MODEL_ID);
		}
		else if (hasBlocker || hasBarricade)
		{
			removeBlockersAt(pos);
		}
	}
}

bool MissionBlockerManager::hasBlockerAt(const Position& t_spawn, int t_modelId) const
{
	float expectedZ = t_modelId == BARRICADE_MODEL_ID ? t_spawn.z + BARRICADE_Z_OFFSET : t_spawn.z;

	for (CObject* blocker : m_blockers)
	{
		if (!blocker || blocker->m_nModelIndex != t_modelId) continue;

		CVector position = blocker->GetPosition();
		float dx = position.x - t_spawn.x;
		float dy = position.y - t_spawn.y;
		float dz = position.z - expectedZ;
		if (dx * dx + dy * dy + dz * dz < BLOCKER_POSITION_TOLERANCE_SQ) return true;
	}
	return false;
}

void MissionBlockerManager::spawnBlocker(const Position& t_pos, int t_modelId)
{
	CObject* object = CObject::Create(t_modelId);
	if (!object) return;

	bool isBarricade = t_modelId == BARRICADE_MODEL_ID;
	float z = isBarricade ? t_pos.z + BARRICADE_Z_OFFSET : t_pos.z;
	object->SetPosition(CVector(t_pos.x, t_pos.y, z));
	object->SetIsStatic(true);
	object->bStreamingDontDelete = true;
	object->bDistanceFade = true;
	if (!isBarricade) object->bIsVisible = false;
	object->m_nObjectType = OBJECT_MISSION;
	CWorld::Add(object);
	m_blockers.push_back(object);
}

void MissionBlockerManager::removeBlockersAt(const Position& t_pos)
{
	size_t liveCount = 0;
	for (CObject* blocker : m_blockers)
	{
		if (!CPools::ms_pObjectPool->IsObjectValid(blocker)) continue;

		int modelId = blocker->m_nModelIndex;
		float expectedZ = modelId == BARRICADE_MODEL_ID ? t_pos.z + BARRICADE_Z_OFFSET : t_pos.z;
		CVector position = blocker->GetPosition();
		float dx = position.x - t_pos.x;
		float dy = position.y - t_pos.y;
		float dz = position.z - expectedZ;
		if (dx * dx + dy * dy + dz * dz < BLOCKER_POSITION_TOLERANCE_SQ)
		{
			CWorld::Remove(blocker);
			delete blocker;
			continue;
		}
		m_blockers[liveCount++] = blocker;
	}
	m_blockers.resize(liveCount);
}

bool MissionBlockerManager::ownsBlocker(const CObject* t_object) const
{
	for (const CObject* blocker : m_blockers)
	{
		if (blocker == t_object) return true;
	}
	return false;
}

bool MissionBlockerManager::isBlockerPosition(const CVector& t_position, int t_modelId) const
{
	for (const Position& spawn : missionStartPos)
	{
		float expectedZ = t_modelId == BARRICADE_MODEL_ID ? spawn.z + BARRICADE_Z_OFFSET : spawn.z;
		float dx = t_position.x - spawn.x;
		float dy = t_position.y - spawn.y;
		float dz = t_position.z - expectedZ;
		if (dx * dx + dy * dy + dz * dz < BLOCKER_POSITION_TOLERANCE_SQ) return true;
	}
	return false;
}

void MissionBlockerManager::adoptExisting()
{
	auto* pool = CPools::ms_pObjectPool;
	if (!pool) return;

	for (int i = 0; i < pool->m_nSize; ++i)
	{
		CObject* object = pool->GetAt(i);
		if (!object) continue;

		int modelId = object->m_nModelIndex;
		if (modelId != BLOCKER_MODEL_ID && modelId != BARRICADE_MODEL_ID) continue;
		if (!isBlockerPosition(object->GetPosition(), modelId)) continue;

		if (!ownsBlocker(object))
		{
			m_blockers.push_back(object);
		}
	}
}

void MissionBlockerManager::removeAll()
{
	for (CObject* blocker : m_blockers)
	{
		if (!CPools::ms_pObjectPool->IsObjectValid(blocker)) continue;

		CWorld::Remove(blocker);
		delete blocker;
	}
	m_blockers.clear();
}

void MissionBlockerManager::forget()
{
	m_blockers.clear();
}
