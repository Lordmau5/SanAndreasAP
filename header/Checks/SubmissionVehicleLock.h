#pragma once
#include <memory>
#include <vector>

class SubmissionTracker;

namespace SubmissionVehicleLock
{
	void update(const std::vector<std::unique_ptr<SubmissionTracker>>& t_trackers);
}
