#include "sol.hpp"
#include "PixelEngineAPI.h" 
#include <unordered_map>
#include "Entity.h" 
#include "PTransform.h"
using namespace ECS;
extern std::unordered_map <std::string, std::function<sol::object(sol::this_state s, Module* target)>> AddModuleList;
inline void Generate_Engine(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Engine");
	ut["CreateEntity"] = &CreateEntity;
	ut["DestroyEntity"] = &DestroyEntity;
	ut["BackgroundColor"] = &BackgroundColor;
}
inline void Generate_Scene(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Scene");
	ut["ChangeScene"] = &ChangeScene;
}
inline void Generate_Asset(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Asset");
	ut["Import"] = &Import;
}
inline void Generate_Input(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Input");
	ut["GetKey"] = &GetKey;
	ut["GetKeyDown"] = &GetKeyDown;
	ut["GetKeyUp"] = &GetKeyUp;
	ut["GetMousePosition_X"] = &GetMousePosition_X;
	ut["GetMousePosition_Y"] = &GetMousePosition_Y;
}
inline void Generate_Debug(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Debug");
	ut["LogInfo"] = &LogInfo;
	ut["LogError"] = &LogError;
	ut["LogWarning"] = &LogWarning;
}
inline void Generate_Transform(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Transform");
	ut["AddTransform"] = &ECS::Transform::AddTransform;
	ut["SetPosition"] = &ECS::Transform::SetPosition;
}
inline void Generate_Entity(sol::state& lua) 
{
	sol::usertype<Entity> ut = lua.new_usertype<Entity>("Entity");
	ut["Active"] = &Entity::Active;
}
inline void BindAll_AddModules() 
{ 
}
inline void BindAll_GeneratedLuaModules(sol::state& lua)
{
	BindAll_AddModules();
	Generate_Engine(lua); 
	Generate_Scene(lua); 
	Generate_Asset(lua); 
	Generate_Input(lua); 
	Generate_Debug(lua); 
	Generate_Transform(lua); 
	Generate_Entity(lua); 
}
