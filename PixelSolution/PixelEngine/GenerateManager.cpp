#include "pch.h"
#include "GenerateManager.h"
#include "PixelMetaAPI.h"
#include "PixelMeta.h"
#include "LuaBind.h"
#include "LSPBind.h"
#include "LuaCreate.h"
using json = nlohmann::json;
void GenerateManager::Initialize()
{
	luaBind = new LuaBind();
	lspBind = new LSPBind();
	luaCreate = new LuaCreate();
}

void GenerateManager::Update()
{

}

void GenerateManager::Release()
{
	delete luaBind;
	delete lspBind;
	delete luaCreate;
}

void GenerateManager::Clear()
{

}

void GenerateManager::CreateBindCode()
{
	types.clear();
	int allCount = GetTypeAllCount();
	for (int typeIndex = 0; typeIndex < allCount; typeIndex++)
	{
		PixelClassMeta PixelClass;
		PType* targetType = GetTypeByIndex(typeIndex);
		PixelClass.metaType = GetTypeCategory(targetType);
		PixelClass.thisName = GetTypeName(targetType);

		if (PixelClass.metaType == META_TYPE::PRIMITIVE)
		{
			//±âº»Çü
		}
		else if (PixelClass.metaType == META_TYPE::CLASS)
		{
			PixelClass.parentHash = GetTypeParentByHash(targetType);
			TypeMember(PixelClass, targetType);
			TypeMethod(PixelClass, targetType);
		}
		else if (PixelClass.metaType == META_TYPE::STATIC)
		{
			TypeMethod(PixelClass, targetType);
		}
		else if (PixelClass.metaType == META_TYPE::ENUM)
		{
			TypeEnum(PixelClass, targetType);
		}
		types.push_back(PixelClass);
	}
}

void GenerateManager::LuaGenerate(const char* outPath)
{
	CreateBindCode();
	luaBind->Generate(outPath, types);
}

void GenerateManager::LSPGenerate(const char* outPath)
{
	CreateBindCode();
	lspBind->Generate(outPath, types);
}

void GenerateManager::JsonGenerate(const char* outPath)
{
	CreateBindCode();
	luaCreate->Generate(outPath, types);
}

void GenerateManager::TypeMember(PixelClassMeta& PixelClass, PType* type)
{
	int memberCount = GetMemberCount(type);
	for (int memberIndex = 0; memberIndex < memberCount; memberIndex++)
	{
		PixelMemberMeta PixelMember;
		PixelMember.name = GetMemberName(type, memberIndex);
		PixelMember.type = GetMemberType(type, memberIndex);
		PixelMember.luaBind = HasMemberFlag(type, memberIndex, MetaFlag::LUABIND);
		PixelClass.members.push_back(PixelMember);
	}
}

void GenerateManager::TypeMethod(PixelClassMeta& PixelClass, PType* type)
{
	int methodCount = GetMethodCount(type);
	for (int methodIndex = 0; methodIndex < methodCount; methodIndex++)
	{
		if (HasMethodFlag(type, methodIndex, MetaFlag::LUABIND) == false) continue;
		PixelMethodMeta PixelMethod;
		PixelMethod.name = GetMethodName(type, methodIndex);
		PixelMethod.returnType = GetMethodReturnType(type, methodIndex);
		int methodPropertyCount = GetMethodPropertyCount(type, methodIndex);
		for (int propertyIndex = 0; propertyIndex < methodPropertyCount; propertyIndex++)
		{
			PixelMethod.propertys.push_back(GetMethodGetPropertyType(type, methodIndex, propertyIndex));
		}
		PixelClass.methods.push_back(PixelMethod);
	}
}

void GenerateManager::TypeEnum(PixelClassMeta& PixelClass, PType* type)
{
	int enumCount = GetEnumCount(type);
	for (int enumIndex = 0; enumIndex < enumCount; enumIndex++)
	{
		PixelEnumMeta PixelEnum;
		PixelEnum.value = GetEnum(type, enumIndex);
		PixelClass.enums.push_back(PixelEnum);
	}
}







