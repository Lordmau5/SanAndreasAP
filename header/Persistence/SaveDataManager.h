#pragma once
#include <string>
#include <unordered_map>

class SaveDataManager
{
public:
	void poll();
	void recordValuesAtSave();
	bool restoreFromCurrentLoadName();
	const std::string& getCurrentSaveKey() const;
	void setValue(const std::string& key, const std::string& value);
	std::string getValue(const std::string& key, const std::string& defaultValue) const;

private:
	std::unordered_map<std::string, std::string> m_values;
	std::unordered_map<std::string, std::string> m_valuesAtSave;
	bool m_hasValuesAtSave = false;
	std::string m_currentSaveKey;
	std::string m_currentSaveDirectory;

	std::string getFilePath() const;
	void loadFromDisk();
	void writeToDisk(const std::unordered_map<std::string, std::string>& t_values) const;
};
