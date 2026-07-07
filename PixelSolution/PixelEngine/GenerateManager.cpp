#include "pch.h"
#include "GenerateManager.h"
#include "PixelMetaAPI.h"
#include "EngineMetaFlag.h"
#include "PixelMeta.h"
#include "LuaComponentCreate.h"
#include "LuaEnumCreate.h"
#include "LuaStaticCreate.h"
#include "LuaTypeCreate.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
using json = nlohmann::json;
void GenerateManager::Initialize()
{
	luaComponentCreate = new LuaComponentCreate();
	luaEnumCreate = new LuaEnumCreate();
	luaStaticCreate = new LuaStaticCreate();
}

void GenerateManager::Update()
{

}

void GenerateManager::Release()
{
	delete luaComponentCreate;
	luaComponentCreate = nullptr;
	delete luaEnumCreate;
	luaEnumCreate = nullptr;
	delete luaStaticCreate;
	luaStaticCreate = nullptr;
}

void GenerateManager::Clear()
{

}

void GenerateManager::CreateBindCode()
{
	PClassMetaList.clear();
	PComponentMetaList.clear();
	PEnumMetaList.clear();


	int allCount = GetTypeAllCount();
	for (int typeIndex = 0; typeIndex < allCount; typeIndex++)
	{
		PType* targetType = GetTypeByIndex(typeIndex);
		auto metaType = GetTypeCategory(targetType);

		if (HasTypeFlag(targetType, EngineMetaFlag::Class))
		{
			AddClass(targetType, PClassMetaList);
		}
		else if (HasTypeFlag(targetType, EngineMetaFlag::ClassData))
		{
			AddClassData(targetType, PClassMetaList);
		}
		else if (HasTypeFlag(targetType, EngineMetaFlag::Component))
		{
			AddClass(targetType, PComponentMetaList);
		}
		else if (HasTypeFlag(targetType, EngineMetaFlag::ComponentData))
		{
			AddClassData(targetType, PComponentMetaList);
		}
		else if (metaType == META_TYPE::ENUM)
		{
			PixelEnumMeta PEnum;
			PEnum.name = GetTypeName(targetType);
			int enumCount = GetEnumCount(targetType);
			PEnum.value = TypeEnum(targetType, enumCount);
			PEnumMetaList.insert({ PEnum.name ,PEnum });
		}
	}
}

void GenerateManager::LuaGenerate(const char* outPath)
{
	//CreateBindCode();
	//luaBind->Generate(outPath, types);
}

void GenerateManager::LSPGenerate(const char* outPath)
{
	//CreateBindCode();
	//lspBind->Generate(outPath, types);
}

void GenerateManager::JsonGenerate(const char* outPath)
{
	CreateBindCode();
	luaEnumCreate->Generate(outPath, PEnumMetaList);
	luaStaticCreate->Generate(outPath, PClassMetaList);
	luaComponentCreate->Generate(outPath, PComponentMetaList);
	//luaStaticCreate->Generate(outPath, types.staticList);
	CreateRequireFile(outPath);
}

void GenerateManager::AddClassData(PType* type, std::map<std::string, PixelClassMeta>& list)
{
	std::string name = GetTypeName(type);
	size_t pos = name.find("Data");
	if (pos != std::string::npos) name.erase(pos, 4);

	pos = name.find("Function");
	if (pos != std::string::npos) name.erase(pos, 8);

	auto find = list.find(name);
	if (find == list.end())
	{
		list.insert({ name ,PixelClassMeta() });
		list[name].name = name;
	}
	int memberCount = GetMemberCount(type);
	list[name].flag = GetTypeFlag(type);
	if (memberCount > 0)
	{
		list[name].members = TypeMember(type, memberCount);
	}
}

void GenerateManager::AddClass(PType* type, std::map<std::string, PixelClassMeta>& list)
{
	std::string name = GetTypeName(type);
	size_t pos = name.find("Function");
	if (pos != std::string::npos) name.erase(pos, 8);

	auto find = list.find(name);
	if (find == list.end())
	{
		list.insert({ name ,PixelClassMeta() });
		list[name].name = name;
	}
	int methodCount = GetMethodCount(type);
	list[name].flag = GetTypeFlag(type);
	if (methodCount > 0)
	{
		list[name].methods = TypeMethod(type, methodCount);
	}
}




void GenerateManager::CreateRequireFile(const std::string& outPath)
{
	std::filesystem::create_directories(outPath);
	std::ofstream file(outPath + "/EngineGenerate.lua");
	file << "require(\"" << "Enum" << "\")\n";
	file << "require(\"" << "PVector2" << "\")\n";
	file << "require(\"" << "PVector3" << "\")\n";

	for (auto& K : PClassMetaList)
	{
		if (K.second.name == "PVector2" || K.second.name == "PVector3")
		{
			continue;
		}
		file << "require(\"" << K.second.name << "\")\n";
	}

	for (auto& K : PComponentMetaList)
	{
		file << "require(\"" << K.second.name << "\")\n";
	}

}
std::vector<PixelMemberMeta> GenerateManager::TypeMember(PType* type, int memberCount)
{
	std::vector<PixelMemberMeta> members;
	for (int memberIndex = 0; memberIndex < memberCount; memberIndex++)
	{
		PixelMemberMeta PixelMember;
		PixelMember.name = GetMemberName(type, memberIndex);
		PixelMember.type = GetMemberType(type, memberIndex);
		PixelMember.luaBind = HasMemberFlag(type, memberIndex, EngineMetaFlag::LuaBind);
		members.push_back(PixelMember);
	}
	return members;
}

std::vector<PixelMethodMeta> GenerateManager::TypeMethod(PType* type, int methodCount)
{
	std::vector<PixelMethodMeta> methods;
	for (int methodIndex = 0; methodIndex < methodCount; methodIndex++)
	{
		PixelMethodMeta PixelMethod;
		PixelMethod.name = GetMethodName(type, methodIndex);
		PixelMethod.returnType = GetMethodReturnType(type, methodIndex);
		PixelMethod.tooltip = GetMethodTooltip(type, methodIndex);
		int methodPropertyCount = GetMethodPropertyCount(type, methodIndex);
		for (int propertyIndex = 0; propertyIndex < methodPropertyCount; propertyIndex++)
		{
			PixelParameterMeta parameter;
			parameter.type = GetMethodGetPropertyType(type, methodIndex, propertyIndex);
			parameter.name = GetMethodGetPropertyName(type, methodIndex, propertyIndex);
			parameter.tooltip = GetMethodGetPropertyTooltip(type, methodIndex, propertyIndex);
			PixelMethod.propertys.push_back(parameter);
		}
		methods.push_back(PixelMethod);
	}
	return methods;
}

std::vector<std::string> GenerateManager::TypeEnum(PType* type, int enumCount)
{
	std::vector<std::string> enums;
	for (int enumIndex = 0; enumIndex < enumCount; enumIndex++)
	{
		enums.push_back(GetEnum(type, enumIndex));
	}
	return enums;
}
