#pragma once
#include <string>

namespace StartingSaves
{
	void setStartingPoint(const std::string& t_name);
	void seedIfNeeded();
	const std::string& missingSaveName();
}
