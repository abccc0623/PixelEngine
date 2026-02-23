#include "BindManager.h"
#include <iostream>
#include <type_traits>
#include <filesystem>
#include <fstream>
#include "GameObject.h"
#include "Transform.h"
#include "LuaScript.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "DebugCamera.h"
#include "Camera.h"
#include "Module.h"
#include "BoxCollision2D.h"
std::string BindManager::apiDefinitions = "";
std::vector<std::string> BindManager::bindClassName = std::vector<std::string>();

template <typename T>
concept CanRegisterLua = requires {
	{ T::RegisterLua() } -> std::same_as<std::string>;
};

template <typename T>
void TryRegister(std::string name)
{
	if constexpr (CanRegisterLua<T>)
	{
		BindManager::apiDefinitions += T::RegisterLua();
	}
	BindManager::bindClassName.push_back(name);
}

#define BIND_MODULE(Type) \
addModuleFunction[#Type] = [](GameObject* Obj) {Obj->AddModule<Type>(); }; \
getLuaModuleFunction.insert({ #Type, [](sol::this_state s, GameObject& obj, std::string name) -> sol::object \
{ \
    auto targetModule = obj.GetModule<Type>(); \
    if (targetModule != nullptr) \
    { \
        return sol::make_object(s,static_cast<Type*>(targetModule.GetPtr())); \
    } \
    return sol::make_object(s, sol::nil); \
} \
}); \
TryRegister<Type>(#Type) \


BindManager::BindManager()
{

}

BindManager::~BindManager()
{

}

void BindManager::Initialize()
{
	BIND_MODULE(Transform);
	BIND_MODULE(LuaScript);
	BIND_MODULE(Camera);
	BIND_MODULE(Renderer2D);
	BIND_MODULE(DebugCamera);
	BIND_MODULE(Collision2D);
	BIND_MODULE(BoxCollision2D);
	CreateLuaAPI_File();
}

void BindManager::Update()
{

}

void BindManager::ReleaseShared()
{

}

void BindManager::AddModuleCall(std::string typeName, GameObject* obj)
{
	auto findTarget = addModuleFunction.find(typeName);
	if (findTarget != addModuleFunction.end())
	{
		findTarget->second(obj);
	}
	else 
	{
		std::cout << "[AddModuleCall]Not Find Module" + typeName << std::endl;
	}
}

sol::object BindManager::GetModuleCall_Lua(sol::this_state s,GameObject& obj, std::string name)
{
	auto target = getLuaModuleFunction.find(name);
	if (target == getLuaModuleFunction.end())
	{
		std::cout << "[GetModuleCall] Not Find Module" << std::endl;
	}
	auto fuc = target->second(s, obj, name);
	return fuc;
}

std::vector<std::string> BindManager::Split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);

	while (std::getline(tokenStream, token, delimiter))
	{
		token.erase(0, token.find_first_not_of(" "));
		token.erase(token.find_last_not_of(" ") + 1);

		if (!token.empty()) {
			tokens.push_back(token);
		}
	}
	return tokens;
}

void BindManager::CreateLuaAPI_File()
{
	std::string apiExportPath = "./Asset/PixelEngine_API.lua";
	std::filesystem::path p = std::filesystem::absolute(apiExportPath); // 절대 경로 확인

	std::filesystem::create_directories(p.parent_path());

	std::ofstream file(p, std::ios::out | std::ios::trunc);
	if (file.is_open()) 
	{
		file << "---@meta PixelEngine API\n\n";
		file << BindManager::apiDefinitions;
		file.flush(); // 버퍼에 남은 데이터를 강제로 파일에 씀
		file.close();
	}
	else 
	{
		std::cerr << "File Open Failed!" << std::endl;
	}
}


