#pragma once

#include "sol.hpp"
namespace ECS::Pool
{
	unsigned int CreateEntity(const char* poolName, const char* scriptName);
	unsigned int Active(const char* poolName);
	void Disable(const char* poolName, unsigned int id);
	void Clear(const char* poolName);

	void SetAutoCreateFunction(const char* poolName, sol::function func);
}