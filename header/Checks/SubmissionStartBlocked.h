#pragma once
#include <memory>
#include <vector>

class SubmissionTracker;

namespace SubmissionStartBlocked
{
	void install(const std::vector<std::unique_ptr<SubmissionTracker>>& t_trackers);
}
