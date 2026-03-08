#pragma once
#include "PixelMetaDLL.h"
#include "PValue.h"
class PClass;
class PType;
class PObject
{
public:
	PIXEL_META_DLL PObject(void* target, PType* type);
	PIXEL_META_DLL ~PObject();

	PIXEL_META_DLL PValue Get(std::string memberName);
	PIXEL_META_DLL void Set(std::string memberName, PValue value);

	PIXEL_META_DLL int GetFieldMaxCount();
	PIXEL_META_DLL int GetMethodMaxCount();

	PIXEL_META_DLL const std::string& TypeName();
	PIXEL_META_DLL const std::string& ParentName();
	PIXEL_META_DLL const std::string& ChildName();
	PIXEL_META_DLL const std::string& GetFieldName(int index);
	PIXEL_META_DLL const std::string& GetFieldType(int index);
private:
	void* target;
	PClass* Classtype;
};

