#pragma once

#include <vector>
#include <sol/forward.hpp>
namespace ECS::Pool
{
	unsigned int Active(const char* scriptName);
	void Disable(const char* scriptName, unsigned int id);
	void Clear(const char* scriptName);

	sol::as_table_t<std::vector<unsigned int>> GetActiveArray(const char* scriptName);
}
