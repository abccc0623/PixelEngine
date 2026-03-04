#pragma once
#include <string>
#include <vector>
#include "GlobalMetaFunc.h"
#include "PixelMetaDLL.h"
#include "PClass.h"
#include "PClassTemplate.h"
#include "PField.h"

class PClass;
class PField;

extern "C" PIXEL_META_DLL PClass* FindClassByName(std::string ClassName);
extern "C" PIXEL_META_DLL PClass* FindClassByHash(uint64_t ClassHash);

extern "C" PIXEL_META_DLL void PixelMetaRelease();
extern "C" PIXEL_META_DLL void ReflectionClassRegister(uint64_t hash, PClass* targetClass);
extern "C" PIXEL_META_DLL void* PixelInternalAlloc(size_t size);
extern "C" PIXEL_META_DLL void PixelInternalFree(void* ptr);


extern "C" PIXEL_META_DLL PField* CreateMetaType(uint64_t hash, const std::string& fieldName, size_t offset, size_t size);

