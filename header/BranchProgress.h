#pragma once
#include <map>
#include <string>
#include "PersistentState.h"

class BranchProgress : public PersistentState
{
public:
	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;

	void receiveItem(const std::string& t_branch);
	void completeMission(const std::string& t_branch);
	bool isBlocked(const std::string& t_branch) const;

private:
	std::map<std::string, int> m_received;
	std::map<std::string, int> m_completed;
};
