#pragma once
#include <cstddef>
#include <string>

const std::string& branchOfMission(int t_missionId);

int branchRadarSprite(const std::string& t_branch);

const std::string& branchDisplayName(const std::string& t_branch);

inline const char* const missionStartPosBranch[] = {
    nullptr,
    "Ryder",
    "Sweet",
    "OG Loc",
    "Sweet",
    "C.R.A.S.H.",
    nullptr,
    "OG Loc",
    "Cesar",
    "C.R.A.S.H.",
    "Catalina",
    "Catalina",
    "The Truth",
    "The Truth",
    "Catalina",
    "Garage",
    "Woozie",
    "Triads",
    "Loco Syndicate",
    nullptr,
    nullptr,
    nullptr,
    "Toreno",
    "Toreno",
    "Toreno",
    "Four Dragons Casino",
    "Four Dragons Casino",
    "Caligula's Palace",
    "C.R.A.S.H.",
    "C.R.A.S.H.",
    "Madd Dogg",
    "Four Dragons Casino",
    "Return",
};

inline constexpr size_t MISSION_START_POS_BRANCH_COUNT =
    sizeof(missionStartPosBranch) / sizeof(missionStartPosBranch[0]);
