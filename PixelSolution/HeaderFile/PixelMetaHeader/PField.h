#pragma once
#include <string>
#include <unordered_map>
#include "PixelMetaDLL.h"


class PField
{
public:
	PIXEL_META_DLL PField(const std::string& type, const std::string& name, size_t offset);
	PIXEL_META_DLL virtual ~PField();

	PIXEL_META_DLL virtual void SetValue(void* instance, void* valuePtr);
	PIXEL_META_DLL virtual void* GetValuePtr(void* instance);
	PIXEL_META_DLL virtual std::string GetFieldName();
	PIXEL_META_DLL virtual std::string GetFieldType();
public:
	std::string type;
	std::string name;
	size_t offset;
};

