#pragma once
#include <string>
#include <vector>
#include <typeindex>
#include <typeinfo>
#include "PixelMetaDLL.h"
#include "GlobalHashCode.h"
#include "PClass.h"
#include "PObject.h"

enum class Generatelanguage
{
	C_Plus_Plus,
	C_Hash,
	LUA,
};

class PType;
class PObject;
extern "C" PIXEL_META_DLL void PixelMetaRelease();
extern "C" PIXEL_META_DLL void RegisterClassType(PType* type);
extern "C" PIXEL_META_DLL PType* GetTypeByString(std::string name);
extern "C" PIXEL_META_DLL void GenerateClassFile(Generatelanguage type,std::string Path);

extern "C" PIXEL_META_DLL PObject* CreateRObject(void* target,std::string TypeName);


template<typename T>
PType* GetTypeByKeyword()
{
	std::string name = ExtractTypeName<T>();
	return GetTypeByString(name);
}

template<typename ThisType, typename ParentType = void>
PClass<ThisType,ParentType>* MakeClassRegister()
{
	auto findType = GetTypeByKeyword<std::decay_t<ThisType>>();
	if (findType == nullptr)
	{
		auto type = new PClass<ThisType,ParentType>();
		RegisterClassType(type);
		return type;
	}
	return static_cast<PClass<ThisType, ParentType>*>(findType);
}

