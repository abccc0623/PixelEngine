#pragma once

#include <vector>
#include <sol/forward.hpp>
namespace ECS::Pool
{
	unsigned int CreateEntity(const char* poolName, const char* scriptName);
	unsigned int Active(const char* poolName);
	void Disable(const char* poolName, unsigned int id);
	void Clear(const char* poolName);

	void SetAutoCreateFunction(const char* poolName, sol::function func);
	void SetAutoActiveFunction(const char* poolName, sol::function func);
	sol::as_table_t<std::vector<unsigned int>> GetActiveArray(const char* poolName);
}
