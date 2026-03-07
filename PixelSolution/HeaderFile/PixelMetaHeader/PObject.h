#pragma once
#include "PixelMetaDLL.h"
#include "PValue.h"
class PType;
class PObject
{
public:
	PIXEL_META_DLL PObject(void* target, PType* type);
	PIXEL_META_DLL ~PObject();

	PIXEL_META_DLL const char* GetTypeName(PObject* obj);
	PIXEL_META_DLL const char* GetFieldName(int index);
	PIXEL_META_DLL int GetFieldCount();

	PIXEL_META_DLL PValue Get(std::string memberName);
	PIXEL_META_DLL void Set(std::string memberName, PValue value);
private:
	void* target;
	PType* type;
};

