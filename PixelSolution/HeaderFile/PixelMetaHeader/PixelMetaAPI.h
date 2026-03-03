#pragma once
#include <string>
#include <vector>
#include "PixelMetaDLL.h"
#include "PClass.h"
#include "PClassTemplate.h"
#include "PField.h"
#include "PFieldTemplate.h"
#include "PFieldString.h"


extern "C" PIXEL_META_DLL void PixelMetaRelease();
extern "C" PIXEL_META_DLL void ReflectionClassRegister(PClass* ptr);
extern "C" PIXEL_META_DLL void* PixelInternalAlloc(size_t size);
extern "C" PIXEL_META_DLL void PixelInternalFree(void* ptr);