#pragma once
#include "sol.hpp"
#include <vector>

namespace ECS::Group
{
	unsigned int CreateEntity(const char* groupName, const char* scriptName);
	void Set(const char* groupName, unsigned int id);
	void Remove(const char* groupName, unsigned int id);
	void Clear(const char* groupName);
	int Count(const char* groupName);
	unsigned int First(const char* groupName);
	sol::as_table_t<std::vector<unsigned int>> Get(const char* groupName);
}
