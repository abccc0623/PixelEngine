#pragma once
#include <sol/forward.hpp>
#include <vector>
#include "PixelEngineDLL.h"

extern "C" PIXEL_ENGINEDLL unsigned int Group_CreateGroupAndEntity(const char* groupName, const char* scriptName);
extern "C" PIXEL_ENGINEDLL void Group_Set(const char* groupName, unsigned int id);
extern "C" PIXEL_ENGINEDLL void Group_Remove(const char* groupName, unsigned int id);
extern "C" PIXEL_ENGINEDLL void Group_Clear(const char* groupName);
extern "C" PIXEL_ENGINEDLL int Group_Count(const char* groupName);
extern "C" PIXEL_ENGINEDLL unsigned int Group_First(const char* groupName);



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
