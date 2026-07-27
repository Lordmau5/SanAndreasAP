#pragma once
#include <array>
#include <string>
#include <memory>
#include "plugin.h"
#include "PendingChecks.h"
#include "CPickups.h"
#include "CMessages.h"
#include <CStats.h>
#include <CWorld.h>

#include "PersistentState.h"
#include "TagTracker.h"
#include "SnapshotTracker.h"
#include "HorseshoeTracker.h"
#include "SubmissionTracker.h"
#include "ParamedicTracker.h"
#include "FirefighterTracker.h"
#include "VigilanteTracker.h"
#include "BurglaryTracker.h"
#include "TaxiTracker.h"
#include "GymTracker.h"
#include "TruckingTracker.h"
#include "ValetTracker.h"
#include "SchoolTracker.h"
#include "PimpingTracker.h"

enum class CheckEvent
{
	None,
	Mission,
	PickUp,
	Submission
};

class CheckListener : public PersistentState
{
public:
	CheckListener();

	// Persists the claimed-tag bitmap, plus every submission tracker it owns.
	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;

	CheckEvent update();
	std::string getMissionID();
	void submissionCheckWasReceived(int t_submissionID);

	void confirmPickUpSent();
	void confirmMissionSent();

	static bool isStoryMission(int missionId);

	// Every collectible check flows through here - Mod drains this instead of a getPending/confirm
	// pair per kind. Mod also gathers the blip targets off this list.
	const std::array<CollectibleTracker*, 3>& getCollectibles() const { return m_collectibles; }

	// Highlight one tag on the map (the /tag command). Tag-only, so it forwards to the tag tracker.
	void locateTag(int t_index) { m_tagTracker.setLocated(t_index); }

	// TEMPORARY: the GXT key the game last reported and the missions-table index it resolves to,
	// for verifying that table against a running game.
	std::string missionDebugLine() const;

	int getPendingSubmissionId();
	void confirmSubmissionSent();

	// Levelled submissions (Paramedic/Firefighter/Vigilante) send a check per level reached
	// rather than one on completion - see LevelledSubmissionTracker for the slot numbering.
	bool hasPendingSubmissionLevel() const;
	int getPendingSubmissionLevelSlot() const;
	void confirmSubmissionLevelSent();
private:
	static constexpr uintptr_t TAXI_FARES_ADDR = 0xA49C30;
	static constexpr int32_t TAXI_FARES_FOR_COMPLETION = 50;

	// Vanilla grants the infinite sprint reward once $10000 worth of goods has been stolen
	// across burglary runs (STAT_MONEY_MADE_FROM_BURGLARY tracks it).
	static constexpr float BURGLARY_LOOT_FOR_COMPLETION = 10000.0f;

	int* m_pickUpCounter;
	int m_lastValuePickUpCounter;

	std::vector<std::string> missions;
	std::string currentMission;
	std::string lastMission;
	int const NO_MISSION = -1;
	// Declared before the trackers so it outlives them - the gym trackers hold a reference to it.
	FightingStyleArbiter m_styleArbiter;
	std::vector<std::unique_ptr<SubmissionTracker>> submissionTrackers;

	bool m_baselinesInitialized = false;

	TagTracker m_tagTracker;
	SnapshotTracker m_snapshotTracker;
	HorseshoeTracker m_horseshoeTracker;
	// All of them, driven uniformly through the interface - declared after the members it points at.
	std::array<CollectibleTracker*, 3> m_collectibles{
		&m_tagTracker, &m_snapshotTracker, &m_horseshoeTracker };

	PendingChecks<int> m_pendingPickUps;
	PendingChecks<std::string> m_pendingMissions;
	PendingChecks<int> m_pendingSubmissions;
	PendingChecks<int> m_pendingSubmissionLevels;

	// Re-takes every "what did this counter read last tick" baseline. Detection works by diffing
	// against them, and a freshly loaded save has different stats than the session that was
	// running - without this, the difference reads as progress and fires phantom checks.
	void resyncBaselines();

	SubmissionTracker* findTracker(int t_submissionID);
	bool pickUpChecker();
	bool missionChecker();
	bool submissionLevelChecker();
	void initializeMissionList();
	void enforceSubmissionRewards();
};

