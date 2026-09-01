#pragma once
#include <vector>
#include "Lockable.h"
#include <string>

class SaveDataManager;

class SubmissionTracker : public Lockable
{
public:
	SubmissionTracker(int t_submissionID);
	SubmissionTracker(int t_submissionID, const std::vector<int> t_submissionValidVehicles);
	virtual ~SubmissionTracker() = default;
	virtual void enforceSubmissionReward() = 0;
	void checkWasReceived();
	void submissionWasCompleted();
	int getSubmissionID();
	const std::vector<int> getSubmissionValidVehicles();

	bool getSubmissionCompleted() const;
	void restoreState(bool t_checkReceived, bool t_submissionCompleted);

	virtual void pollNewTierSlots(std::vector<int>& t_outSlots) {}

	virtual bool pollCompletion() { return false; }

	virtual std::string getSentState() const { return ""; }
	virtual void restoreSentState(const std::string& t_state) {}

	void save(SaveDataManager& t_saveData);
	void load(const SaveDataManager& t_saveData);
protected:
	std::string keyPrefix() const;

	const int SUBMISSION_ID;
	const std::vector<int> SUBMISSION_VALID_VEHICLES;
	bool checkReceived = false;
	bool submissionCompleted = false;
};

