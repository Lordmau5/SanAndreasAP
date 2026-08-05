#include "GameStorageHook.h"
#include <Patch.h>

namespace
{
	// Each function has exactly one call site, verified against gta_sa.exe.
	constexpr uintptr_t DO_GAME_SPECIFIC_STUFF_AFTER_LOAD = 0x618E90;
	constexpr uintptr_t AFTER_LOAD_CALL_SITE = 0x005D1A1D;
	constexpr uintptr_t DO_GAME_SPECIFIC_STUFF_BEFORE_SAVE = 0x618F50;
	constexpr uintptr_t BEFORE_SAVE_CALL_SITE = 0x0061907A;

	bool g_loadHappened = false;

	// Function-local so it exists before another translation unit's static init registers into it.
	std::function<void()>& beforeSaveCallback()
	{
		static std::function<void()> callback;
		return callback;
	}

	// Sets a flag and nothing else: this runs mid-load with the world half-built.
	void onLoadFinished()
	{
		reinterpret_cast<void(*)()>(DO_GAME_SPECIFIC_STUFF_AFTER_LOAD)();
		g_loadHappened = true;
	}

	void onBeforeSave()
	{
		GameStorageHook::notifyBeforeSave();
		reinterpret_cast<void(*)()>(DO_GAME_SPECIFIC_STUFF_BEFORE_SAVE)();
	}
}

void GameStorageHook::install()
{
	plugin::patch::RedirectCall(AFTER_LOAD_CALL_SITE, &onLoadFinished);
	plugin::patch::RedirectCall(BEFORE_SAVE_CALL_SITE, &onBeforeSave);
}

bool GameStorageHook::consumeLoadHappened()
{
	bool happened = g_loadHappened;
	g_loadHappened = false;
	return happened;
}

void GameStorageHook::setBeforeSaveCallback(std::function<void()> t_callback)
{
	beforeSaveCallback() = std::move(t_callback);
}

void GameStorageHook::notifyBeforeSave()
{
	std::function<void()>& callback = beforeSaveCallback();
	if (callback) callback();
}
