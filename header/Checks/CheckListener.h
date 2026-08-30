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
#include "BoatSchoolTracker.h"
#include "BikeSchoolTracker.h"
#include "CourierTracker.h"
#include "RaceTracker.h"
#include "SubmissionVehicleLock.h"
#include "SubmissionStartBlocked.h"
#include "ShootingRangeTracker.h"
#include "DrivingSchoolTracker.h"
#include "ExportListTracker.h"
#include "OysterTracker.h"
#include "QuarryTracker.h"
#include "PimpingTracker.h"
#include "GangTerritoryTracker.h"
#include "GlobalFlagTracker.h"
#include "TrainTracker.h"

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

	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;

	CheckEvent update();
	std::string getMissionID();
	void submissionCheckWasReceived(int t_submissionID);
	void submissionUnlockWasReceived(int t_submissionID);
	void collectibleUnlockWasReceived(const std::string& t_checkType);

	void confirmPickUpSent();
	void confirmMissionSent();

	static bool isStoryMission(int missionId);

	const std::array<CollectibleTracker*, 5>& getCollectibles() const { return m_collectibles; }

	void locateCollectible(const std::string& t_type, int t_index);

	void setIncludedCollectibles(const std::string& t_types);

	int getPendingSubmissionId();
	void confirmSubmissionSent();

	bool hasPendingSubmissionLevel() const;
	int getPendingSubmissionLevelSlot() const;
	void confirmSubmissionLevelSent();
private:
	int* m_pickUpCounter;
	int m_lastValuePickUpCounter;

	std::vector<std::string> missions;
	std::string currentMission;
	std::string lastMission;
	int const NO_MISSION = -1;

	static constexpr char ENDING_CUTSCENE_NAME[] = "RIOT4E1";
	static constexpr int END_OF_THE_LINE_ID = 112;
	bool m_endingFired = false;
	bool isEndingCutscenePlaying() const;
	FightingStyleArbiter m_styleArbiter;
	std::vector<std::unique_ptr<SubmissionTracker>> submissionTrackers;

	bool m_baselinesInitialized = false;

	TagTracker m_tagTracker;
	SnapshotTracker m_snapshotTracker;
	HorseshoeTracker m_horseshoeTracker;
	ExportListTracker m_exportTracker;
	OysterTracker m_oysterTracker;
	std::array<CollectibleTracker*, 5> m_collectibles{
		&m_tagTracker, &m_snapshotTracker, &m_horseshoeTracker, &m_exportTracker,
		&m_oysterTracker };

	PendingChecks<int> m_pendingPickUps;
	PendingChecks<std::string> m_pendingMissions;
	PendingChecks<int> m_pendingSubmissions;
	PendingChecks<int> m_pendingSubmissionLevels;

	void resyncBaselines();

	SubmissionTracker* findTracker(int t_submissionID);
	bool pickUpChecker();
	bool missionChecker();
	bool submissionLevelChecker();
	void initializeMissionList();
	void enforceSubmissionRewards();
};

