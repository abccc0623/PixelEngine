#include "Module.h"
#include <string>
#include <iostream>
#include "PixelEngine.h"

extern PixelEngine* Engine;
Module::Module()
{
	
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

sol::state* Module::GetLua()
{
	return Engine->GetLua();
}


