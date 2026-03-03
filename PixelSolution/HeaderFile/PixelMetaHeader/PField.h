#pragma once
#include <string>
#include <unordered_map>
#include "PixelMetaDLL.h"

class PField
{
public:
	PIXEL_META_DLL PField();
	PIXEL_META_DLL virtual ~PField();

	PIXEL_META_DLL virtual void SetValue(void* instance, void* valuePtr);
	PIXEL_META_DLL virtual void* GetValuePtr(void* instance);
public:
	std::string type;
	std::string name;
	size_t offset;
};

