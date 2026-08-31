#pragma once
#include <string>

namespace SaveRedirect
{
	void install();
	void setWorld(const std::string& t_worldId);
	bool isActive();
	const std::string& directory();
	std::string saveFileName(int t_slot);
}
