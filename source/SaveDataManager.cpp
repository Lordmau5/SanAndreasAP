#include "SaveDataManager.h"
#include <CGenericGameStorage.h>
#include <fstream>
#include <shlobj.h>

#pragma comment(lib, "Shell32.lib")

static std::string extractFileName(const std::string& pathOrName)
{
	size_t pos = pathOrName.find_last_of("\\/");
	return pos == std::string::npos ? pathOrName : pathOrName.substr(pos + 1);
}

void SaveDataManager::recordValuesAtSave()
{
	m_valuesAtSave = m_values;
	m_hasValuesAtSave = true;
}

void SaveDataManager::poll()
{
	std::string saveName = extractFileName(CGenericGameStorage::ms_SaveFileNameJustSaved);

	// Keyed off our own record rather than the name changing: saving twice to the same slot
	// leaves the name identical, and that save still has to reach the companion file.
	if (!m_hasValuesAtSave || saveName.empty()) return;

	m_hasValuesAtSave = false;
	m_currentSaveKey = saveName;
	writeToDisk(m_valuesAtSave);
}

bool SaveDataManager::restoreFromCurrentLoadName()
{
	std::string loadName = extractFileName(CGenericGameStorage::ms_LoadFileName);
	if (loadName.empty()) return false;

	m_currentSaveKey = loadName;
	loadFromDisk();
	return true;
}

const std::string& SaveDataManager::getCurrentSaveKey() const
{
	return m_currentSaveKey;
}

void SaveDataManager::setValue(const std::string& key, const std::string& value)
{
	auto it = m_values.find(key);
	if (it != m_values.end() && it->second == value) return;

	m_values[key] = value;
}

std::string SaveDataManager::getValue(const std::string& key, const std::string& defaultValue) const
{
	auto it = m_values.find(key);
	return it != m_values.end() ? it->second : defaultValue;
}

std::string SaveDataManager::getFilePath() const
{
	char documentsPath[MAX_PATH] = {};
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, documentsPath);
	return std::string(documentsPath) + "\\GTA San Andreas User Files\\" + m_currentSaveKey + "_ap.dat";
}

void SaveDataManager::loadFromDisk()
{
	m_values.clear();

	std::ifstream file(getFilePath());
	if (!file.is_open()) return;

	std::string line;
	while (std::getline(file, line))
	{
		size_t eq = line.find('=');
		if (eq == std::string::npos) continue;
		m_values[line.substr(0, eq)] = line.substr(eq + 1);
	}
}

void SaveDataManager::writeToDisk(const std::unordered_map<std::string, std::string>& t_values) const
{
	std::ofstream file(getFilePath(), std::ios::trunc);
	if (!file.is_open()) return;

	for (const auto& [key, value] : t_values)
	{
		file << key << "=" << value << "\n";
	}
}
