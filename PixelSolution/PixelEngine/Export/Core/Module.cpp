#include "pch.h"
#include "Module.h"
#include "Core/PixelObject.h"
#include "Core/GameObject.h"
#include "PixelEngine.h"
#include "LuaManager.h"
#include "SerializeHelper.h"
extern PixelEngine* Engine;
Module::Module()
{
	targetObject = nullptr;
	type = MODULE_TYPE::NONE;
}
Module::~Module()
{
	targetObject = nullptr;
	type = MODULE_TYPE::NONE;
}

void Module::Awake(){}
void Module::Start(){}
void Module::Update(){}
void Module::MatrixUpdate(){}
void Module::PhysicsUpdate(){}
void Module::LastUpdate(){}

MODULE_TYPE Module::GetType()
{
	return type;
}
GameObject* Module::GetGameObject()
{
	if (targetObject != nullptr)
	{
		return targetObject;
	}
	return nullptr;
}
//void Module::OnCollisionEnter2D(WPointer<GameObject> target){}
//void Module::OnCollision2D(WPointer<GameObject> target){}
//void Module::OnCollisionExit2D(WPointer<GameObject> target){}

//std::string Module::Save(int tab)
//{
//	std::string content = GetTabs(tab) + "{\n";
//
//	// 1. 객체의 기본 정보 저장
//	content += AddEntry(tab + 1, "Name", GetClassNameString()); // GameObject 이름
//
//	return content;
//}


//sol::state* Module::GetLuaState()
//{
//	if (lua == nullptr)
//	{
//		lua = Engine->GetFactory<LuaManager>();
//	}
//	return lua->GetModuleCall_Lua();
//}
//
//void Module::AddLuaAPI(std::string className, std::vector<std::string> functionName)
//{
//	if (lua == nullptr)
//	{
//		lua = Engine->GetFactory<LuaManager>();
//	}
//	lua->AddLuaAPI(className, functionName);
//}
