#pragma once
#include <string>
#include <vector>
#include <typeindex>
#include <typeinfo>
#include "PixelMetaDLL.h"
#include "GlobalHashCode.h"
#include "PClassTemplate.h"
#include "PEnumTemplate.h"
#include "PObject.h"

enum class Generatelanguage
{
	C_Plus_Plus,
	C_Hash,
	LUA,
};


class PType;
class PObject;
class PEnum;
extern "C" PIXEL_META_DLL void PixelMetaRelease();
extern "C" PIXEL_META_DLL void RegisterClassType(PType* type);
extern "C" PIXEL_META_DLL PType* GetTypeByString(std::string name);
extern "C" PIXEL_META_DLL void GenerateClassFile(Generatelanguage type,std::string Path);

extern "C" PIXEL_META_DLL PObject* CreateRObject(void* target,std::string TypeName);
extern "C" PIXEL_META_DLL int PixelMeta_GetFieldMaxCount(PObject* target);
extern "C" PIXEL_META_DLL int PixelMeta_GetMethodMaxCount(PObject* target);
extern "C" PIXEL_META_DLL const char* PixelMeta_TypeName(PObject* target);
extern "C" PIXEL_META_DLL const char* PixelMeta_ParentName(PObject* target);
extern "C" PIXEL_META_DLL const char* PixelMeta_ChildName(PObject* target);
extern "C" PIXEL_META_DLL const char* PixelMeta_GetFieldName(PObject* target,int index);
extern "C" PIXEL_META_DLL const char* PixelMeta_GetFieldType(PObject* target,int index);



template<typename T>
PType* GetTypeByKeyword()
{
	std::string name = ExtractTypeName<T>();
	return GetTypeByString(name);
}

//Class정보를 담을 클래스 생성
template<typename ThisType, typename ParentType = void>
PClassTemplate<ThisType,ParentType>* MakeClassRegister()
{
	auto findType = GetTypeByKeyword<std::decay_t<ThisType>>();
	if (findType == nullptr)
	{
		auto type = new PClassTemplate<ThisType,ParentType>();
		RegisterClassType(type);
		return type;
	}
	return static_cast<PClassTemplate<ThisType, ParentType>*>(findType);
}

//Enum정보를 담을 클래스 생성
template<typename ThisType>
PEnum* MakeEnumRegister() 
{
	auto findType = GetTypeByKeyword<std::decay_t<ThisType>>();
	if (findType == nullptr)
	{
		auto type = new PEnumTemplate<ThisType>();
		RegisterClassType(type);
		return type;
	}
	return static_cast<PEnumTemplate<ThisType>*>(findType);
}
