#include "Module.h"
#include "PixelObject.h"
#include "GameObject.h"
#include <string>
#include <iostream>
#include "PixelEngine.h"
#include "LuaManager.h"
#include "SerializeHelper.h"
extern PixelEngine* Engine;
LuaManager* Module::lua = nullptr;
Module::Module()
{
	targetObject = nullptr;
}
Module::~Module()
{

}

void Module::Awake(){}
void Module::Start(){}
void Module::Update(){}
void Module::MatrixUpdate(){}
void Module::PhysicsUpdate(){}
void Module::LastUpdate(){}
void Module::OnCollisionEnter2D(WPointer<GameObject> target){}
void Module::OnCollision2D(WPointer<GameObject> target){}
void Module::OnCollisionExit2D(WPointer<GameObject> target){}

std::string Module::Save(int tab)
{
	std::string content = GetTabs(tab) + "{\n";

	// 1. 객체의 기본 정보 저장
	content += AddEntry(tab + 1, "Name", GetClassNameString()); // GameObject 이름

	return content;
}

std::string Module::GetClassNameString()
{
	if (className.empty())
	{
		std::string name = typeid(*this).name();
		if (name.find("class ") == 0) name = name.substr(6);
		if (name.find("struct ") == 0) name = name.substr(7);
		className = name;
	}
	return className;
}


sol::state* Module::GetLuaState()
{
	if (lua == nullptr)
	{
		lua = Engine->GetFactory<LuaManager>();
	}
	return lua->GetModuleCall_Lua();
}

void Module::AddLuaAPI(std::string className, std::vector<std::string> functionName)
{
	if (lua == nullptr)
	{
		lua = Engine->GetFactory<LuaManager>();
	}
	lua->AddLuaAPI(className, functionName);
}
