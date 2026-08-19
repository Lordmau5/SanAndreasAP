#pragma once
#include <string>
#include <unordered_map>

// Persists arbitrary AP mod state (progressive mission counter, submission reward flags, ...)
// to a companion file next to the actual GTA SA save it belongs to, since none of that state
// is part of the vanilla save format and would otherwise reset to defaults every time the game
// process restarts. Values are held as strings so callers don't need a shared schema here.
class SaveDataManager
{
public:
	// Call once per tick: writes the recorded values out once the game has named the slot it
	// just saved to. Loads are deliberately NOT detected here - ms_LoadFileName changes while
	// merely browsing the load menu, so restores are driven by the caller's load signal instead.
	void poll();

	// Call from the game's pre-save hook. Records the current values so the companion file holds
	// what the real save holds, instead of drifting on as the player keeps playing.
	void recordValuesAtSave();

	// Restore path, called when the caller has established a save really was just loaded (the
	// blip-pool wipe signal plus a non-fresh-game check): adopts whatever ms_LoadFileName
	// currently holds as the active save and restores its companion file.
	bool restoreFromCurrentLoadName();

	const std::string& getCurrentSaveKey() const;

	void setValue(const std::string& key, const std::string& value);
	std::string getValue(const std::string& key, const std::string& defaultValue) const;

private:
	std::unordered_map<std::string, std::string> m_values;
	// Taken at pre-save and written once the slot name is known - the game does not name it until
	// the save is done, and no tick of ours runs in between.
	std::unordered_map<std::string, std::string> m_valuesAtSave;
	bool m_hasValuesAtSave = false;

	// The active companion file is keyed off the real save's own filename (e.g. "GTASAsf1.b"),
	// so each of GTA SA's 8 save slots gets independent AP state. Empty until the player has
	// loaded or saved at least once this process - there is no save to associate data with yet.
	std::string m_currentSaveKey;

	std::string getFilePath() const;
	void loadFromDisk();
	void writeToDisk(const std::unordered_map<std::string, std::string>& t_values) const;
};
