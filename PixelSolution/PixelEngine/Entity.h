#pragma once
#include "PixelEngineDLL.h"
extern "C" PIXEL_ENGINEDLL unsigned int Entity_Create(const char* scriptName);
extern "C" PIXEL_ENGINEDLL unsigned int Entity_CreateEmpty();
extern "C" PIXEL_ENGINEDLL void Entity_Destroy(unsigned int id);
extern "C" PIXEL_ENGINEDLL void Entity_SetActive(unsigned int id, bool active);
extern "C" PIXEL_ENGINEDLL bool Entity_GetActive(unsigned int id);

extern "C" PIXEL_ENGINEDLL sol::object Entity_GetValue(unsigned int id, const char* memberName);
extern "C" PIXEL_ENGINEDLL void Entity_SetValue(unsigned int id, const char* memberName, sol::object);


