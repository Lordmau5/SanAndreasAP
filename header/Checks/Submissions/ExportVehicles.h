#pragma once
#include <array>

// The Easter Basin export board, in board order: three lists of ten. Index here is the AP location
// index, so the order must match worlds/gta_sa/export_list.py exactly.
inline constexpr std::array<int, 30> exportVehicleModels = {
	// List 1
	470, 468, 409, 533, 534, 402, 405, 411, 483, 445,
	// List 2
	535, 496, 580, 475, 521, 415, 489, 439, 514, 480,
	// List 3
	536, 463, 500, 477, 587, 429, 506, 508, 579, 424,
};
