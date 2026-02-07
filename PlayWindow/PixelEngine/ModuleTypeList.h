#pragma once

#include "GameObject.h"
#include <functional>
#include <string>

#include "Transform.h"
#include "LuaScript.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "DebugCamera.h"

std::unordered_map<std::string, std::function<void(GameObject*)>> moduleFactories;
#define BIND_MODULE(Type) \
    moduleFactories[#Type] = [](GameObject* obj) { return obj->AddModule<Type>(); }

void ModuleRegister()
{
    BIND_MODULE(Transform);
	BIND_MODULE(LuaScript);
	BIND_MODULE(Renderer);
	BIND_MODULE(Renderer2D);
	BIND_MODULE(DebugCamera);
}






