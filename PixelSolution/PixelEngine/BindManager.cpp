#include "pch.h"
#include "BindManager.h"
#include <iostream>
#include "Core/GameObject.h"
#include "Type/GlobalEnum.h"
#include "Renderer.h"
#include "BoxCollision2D.h"
#include "GlobalBind.h"

#include "Module/Renderer2D.h"
#include "Module/Transform.h"
#include "Module/LuaScript.h"
#include "Module/DebugCamera.h"
#include "Module/Camera.h"

#include "PixelMetaAPI.h"
#include "PMethod.h"

#include "Type/PVector3.h"
#include "Rect.h"


//std::string BindManager::apiDefinitions = "";
//std::vector<std::string> BindManager::bindClassName = std::vector<std::string>();

/*
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
*/


BindManager::BindManager()
{

}

BindManager::~BindManager()
{

}

void BindManager::Initialize()
{
	ModuleTypeBind<Transform>::Bind(MODULE_TYPE::Transform);
	ModuleTypeBind<Camera>::Bind(MODULE_TYPE::Camera);
	ModuleTypeBind<DebugCamera>::Bind(MODULE_TYPE::DebugCamera);
	ModuleTypeBind<LuaScript>::Bind(MODULE_TYPE::LuaScript);
	ModuleTypeBind<Renderer2D>::Bind(MODULE_TYPE::Renderer2D);

	//auto moduleEnum = MakeEnumRegister<MODULE_TYPE>();

	//PEnum* p = MakeEnumRegister("ModuleType");
	//p->AddElement("Transform",0);
	//p->AddElement("Camera",1);
	//p->AddElement("DebugCamera",2);
	//p->AddElement("LuaScript",3);
	//p->AddElement("Renderer2D",4);

	//기본 오브젝트
	auto baseObject = MakeClassRegister<PixelObject>();
	
	//게임 오브젝트
	auto GObject = MakeClassRegister<GameObject, PixelObject>();
	//GObject->AddField("name", &GameObject::name);
 	GObject->AddMethod("test", &GameObject::test);
	GObject->AddMethod("test1", &GameObject::test1);

	GameObject* temp = new GameObject();
	auto k1 = GObject->GetMethod("test");
	auto k2 = GObject->GetMethod("test1");
	std::vector<void*> test = std::vector<void*>();
	int a = 10;
	int b = 20;
	int c = 30;
	test.push_back(&a);
	test.push_back(&b);
	test.push_back(&c);
	k1->Invoke(temp,test);
	
}

void BindManager::Update()
{

}

void BindManager::ReleaseShared()
{

}

//void BindManager::AddModuleCall(std::string typeName, GameObject* obj)
//{
//	//auto findTarget = addModuleFunction.find(typeName);
//	//if (findTarget != addModuleFunction.end())
//	//{
//	//	findTarget->second(obj);
//	//}
//	//else 
//	//{
//	//	std::cout << "[AddModuleCall]Not Find Module" + typeName << std::endl;
//	//}
//}

//sol::object BindManager::GetModuleCall_Lua(sol::this_state s,GameObject& obj, std::string name)
//{
//	//auto target = getLuaModuleFunction.find(name);
//	//if (target == getLuaModuleFunction.end())
//	//{
//	//	std::cout << "[GetModuleCall] Not Find Module" << std::endl;
//	//}
//	//auto fuc = target->second(s, obj, name);
//	return fuc;
//}

//std::vector<std::string> BindManager::Split(const std::string& s, char delimiter)
//{
//	//std::vector<std::string> tokens;
//	//std::string token;
//	//std::istringstream tokenStream(s);
//	//
//	//while (std::getline(tokenStream, token, delimiter))
//	//{
//	//	token.erase(0, token.find_first_not_of(" "));
//	//	token.erase(token.find_last_not_of(" ") + 1);
//	//
//	//	if (!token.empty()) {
//	//		tokens.push_back(token);
//	//	}
//	//}
//	return nullptr;
//}

//void BindManager::CreateLuaAPI_File()
//{
//	std::string apiExportPath = "./Asset/PixelEngine_API.lua";
//	std::filesystem::path p = std::filesystem::absolute(apiExportPath); // 절대 경로 확인
//
//	std::filesystem::create_directories(p.parent_path());
//
//	std::ofstream file(p, std::ios::out | std::ios::trunc);
//	if (file.is_open()) 
//	{
//		file << "---@meta PixelEngine API\n\n";
//		file << BindManager::apiDefinitions;
//		file.flush(); // 버퍼에 남은 데이터를 강제로 파일에 씀
//		file.close();
//	}
//	else 
//	{
//		std::cerr << "File Open Failed!" << std::endl;
//	}
//}


