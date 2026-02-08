#include "GameObject.h"
#include <functional>
#include <string>
#include "sol.hpp"

#include "ModuleTypeList.h"
#include "Transform.h"
#include "LuaScript.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "DebugCamera.h"
#include "Camera.h"
#include "BindLua.h"

std::unordered_map<std::string, std::function<void(GameObject*)>> moduleFactories;
std::unordered_map<std::string, std::function<sol::object(sol::state_view, Module*)>> moduleLuaFactories;

#define BIND_MODULE(Type) \
    moduleFactories[#Type] = [](GameObject* obj) { return obj->AddModule<Type>(); }; \
    moduleLuaFactories[#Type] = [](sol::state_view s, Module* mod) -> sol::object {return sol::make_object(s, static_cast<Type*>(mod));}; \
    if constexpr (std::is_base_of_v<BindLua<Type>, Type>) \
    { \
        BindLua<Type>::RegisterLua(); \
    }


void ModuleRegister()
{
	BIND_MODULE(Transform);
	BIND_MODULE(LuaScript);
	BIND_MODULE(Renderer);
	BIND_MODULE(Renderer2D);
	BIND_MODULE(DebugCamera);
	BIND_MODULE(Camera);
}
