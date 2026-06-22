#pragma once

#include <sol/forward.hpp>
namespace ECS::Entity
{
	sol::object GetValue(unsigned int id, const char* memberName);
	void SetValue(unsigned int id, const char* memberName, sol::object);
}