#pragma once
#include <vector>
#include "Position.h"

inline constexpr int LOS_SANTOS_GYM_ID = 114;
inline constexpr int SAN_FIERRO_GYM_ID = 115;
inline constexpr int LAS_VENTURAS_GYM_ID = 116;
inline constexpr int TAXI_ID = 121;
inline constexpr int PARAMEDIC_ID = 122;
inline constexpr int FIREFIGHTER_ID = 123;
inline constexpr int VIGILANTE_ID = 124;
inline constexpr int BURGLARY_ID = 125;
inline constexpr int TRUCKING_ID = 117;
inline constexpr int PIMPING_ID = 127;
inline constexpr int BOAT_SCHOOL_ID = 119;
inline constexpr int BIKE_SCHOOL_ID = 120;
inline constexpr int QUARRY_ID = 118;
inline constexpr int VALET_ID = 136;
inline constexpr int DRIVING_SCHOOL_ID = 137;
inline constexpr int FLYING_SCHOOL_ID = 138;
inline constexpr int GANG_TERRITORY_ID = 139;

inline constexpr int EIGHT_TRACK_ID = 140;
inline constexpr int DIRT_TRACK_ID = 141;
inline constexpr int BLOOD_RING_ID = 142;
inline constexpr int KICKSTART_ID = 143;

inline constexpr int DRIVING_SCHOOL_SCORE_GLOBALS[] = {
	91, 92, 94, 96, 97, 98, 100, 101, 102, 103, 105, 107,
};
inline constexpr int FLYING_SCHOOL_SCORE_GLOBALS[] = {
	1942, 1943, 1944, 1945, 1946, 1947, 1948, 1949, 1950, 1951,
};
inline constexpr int BIKE_SCHOOL_SCORE_GLOBALS[] = {
	2164, 2165, 2166, 2167, 2168, 2169,
};
inline constexpr int BOAT_SCHOOL_SCORE_GLOBALS[] = {
	1963, 1964, 1965, 1966, 1967,
};

inline constexpr int CHILIAD_CHALLENGE_ID = 132;
inline constexpr int CHILIAD_PASSED_GLOBAL = 1801;

inline constexpr int BLOOD_RING_PASSED_GLOBAL = 1941;
inline constexpr int KICKSTART_PASSED_GLOBAL = 90;

inline constexpr int OPTIONAL_MISSION_IDS[] = {
	67, 68, 69, 70,   // Wang Cars: Zeroing In, Test Drive, Customs Fast Track, Puncture Wounds

	71,               // Driving School: Back to School
	72, 73, 74,       // Zero: Air Raid, Supply Lines, New Model Army

	96, 97, 98, 99, 100, 101,

	140, 141, 142, 143,   // Stadium: 8-Track, Dirt Track, Blood Ring, Kickstart
};

struct SubmissionTierSpec
{
	int baseSlot;
	int tierCount;
	float progressPerTier;
};

inline constexpr SubmissionTierSpec PARAMEDIC_TIERS   { 0,  12, 1.0f };
inline constexpr SubmissionTierSpec FIREFIGHTER_TIERS { 12, 12, 1.0f };
inline constexpr SubmissionTierSpec VIGILANTE_TIERS   { 24, 12, 1.0f };
inline constexpr SubmissionTierSpec TAXI_TIERS        { 36, 50, 1.0f };
inline constexpr SubmissionTierSpec BURGLARY_TIERS    { 86, 10, 1000.0f };
inline constexpr SubmissionTierSpec TRUCKING_TIERS    { 96,  8, 1.0f };
inline constexpr SubmissionTierSpec VALET_TIERS       { 104,  5, 0.0f };
inline constexpr SubmissionTierSpec DRIVING_SCHOOL_TIERS { 109, 12, 1.0f };
inline constexpr SubmissionTierSpec PIMPING_TIERS     { 121, 10, 1.0f };
inline constexpr SubmissionTierSpec FLYING_SCHOOL_TIERS { 131, 10, 1.0f };
inline constexpr SubmissionTierSpec BOAT_SCHOOL_TIERS  { 141, 5, 1.0f };
inline constexpr SubmissionTierSpec BIKE_SCHOOL_TIERS  { 146, 6, 1.0f };
inline constexpr SubmissionTierSpec QUARRY_TIERS       { 152, 7, 1.0f };
inline constexpr SubmissionTierSpec GANG_TERRITORY_TIERS { 159, 20, 5.0f };

inline constexpr int SUBMISSION_TIER_SLOT_COUNT = 179;

inline std::vector<Position> missionStartPos = {
    { 700, -3328, 20, 180 },
    { 2459.55, -1687.75, 12.56, 0 },
    { 2515.0701, -1673.98, 12.71, 0 },
    { 2486.6101, -1649.42, 12.48, 0 },
    { 1365.2507, -1280.12, 12.5469, 0 },
    { 1042.85, -1338.62, 12.55, 0 },
    { 2070.8701, -1703.01, 12.55, 0 },
    { 790.54, -1627.91, 12.39, 0 },
    { 1801.08, -2117.9199, 12.56, 0 },
    { -2043.34, -2525.99, 29.62, 0 },
    { 681.595, -478.7909, 15.3281, 0 },
    { 868.3358, -29.5529, 62.3276, 0 },
    { -2198.8696, -2261.2024, 29.6419, 0 },
    { -922.5121, -1719.3951, 76.5703, 0 },
    { -513.9356, -188.314, 77.4599, 0 },
    { -2030.4019, 148.8279, 27.8359, 0 },
    { -2154.208, 645.3251, 51.3516, 0 },
    { -1717.05, 1280.91, 6.23, 0 },
    { -2623.4971, 1405.6602, 6.1016, 0 },
    { -2031.2612, 179.2488, 27.8359, 0 },
    { -2031.4, -116.5, 1034.1, 0 },
    { -2245.6631, 128.8889, 34.3203, 0 },
    { -685.2156, 923.2191, 11.1531, 0 },
    { 327.448, 2530.095, 15.8066, 0 },
    { 415.55, 2533.5701, 19.18, 0 },
    { 1962.4316, 974.675, 993.4688, 0 },
    { 2026.6028, 1007.7353, 9.8127, 0 },
    { 2270.6335, 1635.5992, 1007.3672, 0 },
    { 1598.5573, 2667.8296, 9.8203, 0 },
    { -378.75, 2235.8501, 41.42, 0 },
    { 2090.0, 1451.0, 9.8, 0 },
    { 1253.788, -785.2594, 91.0313, 0 },
    { 1552.7803, 39.3031, 23.1445, 0 },
    { 2495.2144, -1687.0298, 12.5144, 0 },
};
