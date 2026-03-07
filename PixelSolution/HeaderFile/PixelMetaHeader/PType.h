#pragma once
#include <string>
#include <functional>
#include "PixelMetaDLL.h"
class PField;
class PMethod;
class PType
{
public:
	PIXEL_META_DLL PType(uint64_t hash, std::string name, size_t size);
	PIXEL_META_DLL PType();
	PIXEL_META_DLL virtual ~PType();

	PIXEL_META_DLL std::string GetName();
	PIXEL_META_DLL uint64_t GetHash();
	PIXEL_META_DLL uint64_t GetSize();
	PIXEL_META_DLL virtual PField* GetField(const std::string& name);
	PIXEL_META_DLL virtual PMethod* GetMethod(const std::string& name);


	PIXEL_META_DLL bool operator==(const PType& other);
	PIXEL_META_DLL bool operator!=(const PType& other);
	PIXEL_META_DLL static void* operator new(size_t size);
	PIXEL_META_DLL static void operator delete(void* ptr);
protected:
	uint64_t typeHash;
	std::string typeName;
	size_t typeSize;
};

