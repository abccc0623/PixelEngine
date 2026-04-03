#pragma once
#include <string>
#include "PixelMetaDLL.h"
#include "GlobalHashCode.h"

class PField;
class PMethod;
class PType
{
public:
	PType(uint64_t hash, std::string name, size_t size);
	PType();
	virtual ~PType();

	PIXEL_META_DLL const std::string& GetName();
	int GetMetaType();
	uint64_t GetHash();
	uint64_t GetSize();

	bool operator==(const PType& other);
	bool operator!=(const PType& other);
	static void* operator new(size_t size);
	static void operator delete(void* ptr);
protected:
	uint64_t typeHash;
	std::string typeName;
	size_t typeSize;
	META_TYPE metaType;
};

