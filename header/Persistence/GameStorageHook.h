#pragma once
#include <functional>

// Reliable save/load signals, taken from the game itself.
namespace GameStorageHook
{
	// Call once at startup. Patches both call sites; the originals still run.
	void install();

	// True once per load, cleared by reading it. The load is over by the time this is seen, so
	// the caller can do its work on a normal tick.
	bool consumeLoadHappened();

	// True once per New Game started from the menu (not a load), cleared by reading it. Lets the
	// caller wipe stale per-slot state instead of carrying the previous save's into the new one.
	bool consumeNewGameHappened();

	// Runs INSIDE the game's pre-save, before anything is serialised - there is no tick between
	// that moment and the write, so anything that must reach the file has to happen here.
	void setBeforeSaveCallback(std::function<void()> t_callback);

	// Call before saving the game ourselves. The patched call site sits in the pause-menu code
	// upstream of GenericSave, so a save we start by calling GenericSave directly never reaches it.
	void notifyBeforeSave();
}
