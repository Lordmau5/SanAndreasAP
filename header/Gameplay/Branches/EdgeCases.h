#pragma once
#include "BranchController.h"

// Body Harvest puts King in Exile's market on the spot where badlands is, so it has to be deleted first
class BcrashController : public EdgeCase
{
public:
	BcrashController();

	void update() override;
};

class CatController : public EdgeCase
{
public:
	CatController();

	void update() override;

private:
	static constexpr int SPOT_STRIDE = 24;
	static constexpr int FIRST_DATE_OFFSET = 2044;
	static constexpr int CABIN_OFFSET = 2024;
	static constexpr int CABIN_SPRITE_OFFSET = 1948;
	static constexpr int BLIP_HANDLE_OFFSET = 1932;

	Marker firstDate() const;
	void writeSkippedMissionState() const;
};

class TruController : public EdgeCase
{
public:
	TruController();

	void update() override;

private:
	static constexpr int SAN_FIERRO_OFFSET = 1988;
	static constexpr int BCESAR_COUNTER_OFFSET = 1968;
	static constexpr int BCESAR_FINISHED = 10;
};

// Wu Zi Mu and Farewell, My Love. One mission script re-entered at five counter stages with two
// races between, and the second marker is a literal the script holds in no global.
class BcesarController : public EdgeCase
{
public:
	BcesarController();

	void update() override;

private:
	static constexpr int CATALINA_COUNTER_OFFSET = 256;
	static constexpr int CATALINA_ROBBERIES_DONE = 4;
	static constexpr int FAREWELL_FIRST = 5;
	static constexpr int FAREWELL_LAST = 7;
};
