#pragma once

class AutoSaveManager
{
public:
	void requestSave();
	bool update();

private:
	static constexpr int AUTOSAVE_SLOT = 8;
	static constexpr long TITLE_OFFSET = 9;
	static constexpr int TITLE_SIZE = 100;

	static constexpr int VALID_SAVE_NAME_SIZE = 256;

	bool isMissionScriptActive() const;
	bool performSave();
	void patchSaveTitle() const;

	bool m_savePending = false;
};
