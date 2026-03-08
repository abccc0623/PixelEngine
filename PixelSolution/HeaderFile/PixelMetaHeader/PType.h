#pragma once
#include <string>
#include "PixelMetaDLL.h"

enum class META_TYPE
{
	PRIMITIVE,
	CLASS,
	ENUM,
	ARRAY,
};

class PField;
class PMethod;
class PType
{
public:
	PIXEL_META_DLL PType(uint64_t hash, std::string name, size_t size);
	PIXEL_META_DLL PType();
	PIXEL_META_DLL virtual ~PType();

	PIXEL_META_DLL const std::string& GetName();
	PIXEL_META_DLL const std::string& GetMetaType();
	PIXEL_META_DLL uint64_t GetHash();
	PIXEL_META_DLL uint64_t GetSize();

	PIXEL_META_DLL bool operator==(const PType& other);
	PIXEL_META_DLL bool operator!=(const PType& other);
	PIXEL_META_DLL static void* operator new(size_t size);
	PIXEL_META_DLL static void operator delete(void* ptr);
protected:
	uint64_t typeHash;
	std::string typeName;
	size_t typeSize;
	META_TYPE metaType;
};

