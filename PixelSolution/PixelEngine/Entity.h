#pragma once
#include "PixelEngineDLL.h"
struct UIImageData;
extern "C" PIXEL_ENGINEDLL unsigned int Entity_Create(const char* scriptName);
extern "C" PIXEL_ENGINEDLL unsigned int Entity_CreateEmpty();
extern "C" PIXEL_ENGINEDLL unsigned int Entity_CreateCamera();
extern "C" PIXEL_ENGINEDLL UIImageData* Entity_Create2DImage(const char* textureName);

extern "C" PIXEL_ENGINEDLL void Entity_Destroy(unsigned int id);
extern "C" PIXEL_ENGINEDLL void Entity_SetActive(unsigned int id, bool active);
extern "C" PIXEL_ENGINEDLL bool Entity_GetActive(unsigned int id);
