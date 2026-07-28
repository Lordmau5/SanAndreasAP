#pragma once
#include <vector>
#include "BlipTarget.h"
#include "PersistentState.h"

class SaveDataManager;

// Draws collectible blips (spray tags, snapshots) on the radar/map plus their 1-based numbers on the
// minimap, and owns the invisible world-wipe sentinel. It does NO selection: each tick it is handed
// the full list of collectibles, already ranked by distance (see rankByDistance), and just makes the
// radar match - so the shared nearest-N budget across every collectible set is decided upstream, in
// one place, instead of each set fighting over the 175-trace pool separately.
class CollectibleBlipsManager : public PersistentState
{
public:
	// Persists the on/off toggle, so the player's preference survives a restart per save slot.
	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;

	// Call once per tick, AFTER onWorldWiped() when a wipe was detected. Takes ownership of the
	// targets (kept for drawNumbers). Returns true when a world wipe was just observed - the blip
	// pool only ever resets on a real load or new game, which makes this the mod's authoritative
	// "world was reloaded" signal (unlike ms_LoadFileName, which also changes while merely browsing
	// the load menu).
	bool render(std::vector<BlipTarget> t_targets);

	// Call from the HUD draw event, after the native HUD (and radar) have drawn.
	void drawNumbers() const;

	// Call the moment a world wipe (load / new game) is detected, BEFORE render(). Blips are stored
	// in the save file, so a load brings our old ones back while leaving our handles dangling -
	// recreating on top of them would double the blip count every single load.
	void onWorldWiped();

	// Player preference (pause-menu toggle): disabling clears every collectible blip and stops
	// drawing numbers. The wipe-detection sentinel stays alive regardless.
	void toggleBlips();
	bool areBlipsEnabled() const;

private:
	// Index of the target at t_pos, or -1. Position alone does not prove a blip is ours.
	int targetIndexAt(const CVector& t_pos) const;
	// Index of the target a blip at t_pos with t_sprite is ours for, or -1. Needs both: our sprites
	// are shared with vanilla (Pay 'n' Spray, question-mark missions) and so are some positions.
	int ownedTargetIndexAt(const CVector& t_pos, int t_sprite) const;
	bool ownsBlip(int t_handle) const;
	int findExistingBlipAt(const CVector& t_pos, int t_sprite) const;

	// Rebuilds the handle table from the blips actually present in the world: adopts the ones a save
	// restored and drops duplicates. Makes the blip count idempotent no matter how we arrived.
	void reconcileWithPool();

	std::vector<BlipTarget> m_targets;
	// Aligned with m_targets by index; the target order is stable each tick, so handles persist.
	std::vector<int> m_handles;

	// Counted down over several ticks rather than run once, because a save's radar data may not be
	// in place on the very first tick after a load - a single early pass would find nothing to clean
	// up and duplicates from older saves would survive.
	int m_reconcileTicks = RECONCILE_TICKS;
	static constexpr int RECONCILE_TICKS = 10;

	static constexpr int MAX_BLIPS = 30;
	
	static constexpr float RANGE_HYSTERESIS = 1.15f;

	bool m_blipsEnabled = true;

	// Never-displayed blip whose death is the wipe detector - kept separate from the collectible
	// blips so the signal works even if every one of them is claimed.
	int m_sentinelHandle = -1;
};
