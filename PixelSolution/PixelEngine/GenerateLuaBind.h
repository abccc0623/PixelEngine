#include "sol.hpp"
#include "PixelEngineAPI.h" 
#include <unordered_map>
#include "Core/GameObject.h" 
#include "Module/Transform.h" 
#include "Module/Camera.h" 
#include "Module/LuaScript.h" 
#include "Module/DebugCamera.h" 
#include "Module/Renderer2D.h" 
#include "Type/PVector3.h" 
extern std::unordered_map <std::string, std::function<sol::object(sol::this_state s, Module* target)>> AddModuleList;
inline void Generate_Engine(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Engine");
	ut["CreateGameObject"] = &CreateGameObject;
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
inline void Generate_Vector3(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Vector3");
	ut["Lerp"] = &Lerp;
	ut["Distance"] = &Distance;
}
inline void Generate_GameObject(sol::state& lua) 
{
	sol::usertype<GameObject> ut = lua.new_usertype<GameObject>("GameObject");
	ut["AddModule"] = &GameObject::AddModule;
	ut["GetModule"] = &GameObject::GetModule;
}
inline void Generate_Module(sol::state& lua) 
{
	sol::usertype<Module> ut = lua.new_usertype<Module>("Module");
}
inline void Generate_Transform(sol::state& lua) 
{
	sol::usertype<Transform> ut = lua.new_usertype<Transform>("Transform");
	ut["Position"] = &Transform::Position;
	ut["Rotation"] = &Transform::Rotation;
	ut["Scale"] = &Transform::Scale;
	ut["GetLookVector"] = &Transform::GetLookVector;
	ut["GetRightVector"] = &Transform::GetRightVector;
	ut["GetUpVector"] = &Transform::GetUpVector;
}
inline void Generate_Camera(sol::state& lua) 
{
	sol::usertype<Camera> ut = lua.new_usertype<Camera>("Camera");
}
inline void Generate_LuaScript(sol::state& lua) 
{
	sol::usertype<LuaScript> ut = lua.new_usertype<LuaScript>("LuaScript");
	ut["Register"] = &LuaScript::Register;
}
inline void Generate_DebugCamera(sol::state& lua) 
{
	sol::usertype<DebugCamera> ut = lua.new_usertype<DebugCamera>("DebugCamera");
}
inline void Generate_Renderer2D(sol::state& lua) 
{
	sol::usertype<Renderer2D> ut = lua.new_usertype<Renderer2D>("Renderer2D");
	ut["SetTexture"] = &Renderer2D::SetTexture;
}
inline void Generate_PVector3(sol::state& lua) 
{
	sol::usertype<PVector3> ut = lua.new_usertype<PVector3>("PVector3",sol::constructors<PVector3(float,float,float)>()); 
	ut["X"] = &PVector3::X;
	ut["Y"] = &PVector3::Y;
	ut["Z"] = &PVector3::Z;
}
inline void BindAll_AddModules() 
{ 
	AddModuleList.insert({ "Transform",[](sol::this_state s, Module* target) -> sol::object{sol::object obj = sol::make_object(s, static_cast<Transform* > (target));return obj;}});
	AddModuleList.insert({ "Camera",[](sol::this_state s, Module* target) -> sol::object{sol::object obj = sol::make_object(s, static_cast<Camera* > (target));return obj;}});
	AddModuleList.insert({ "LuaScript",[](sol::this_state s, Module* target) -> sol::object{sol::object obj = sol::make_object(s, static_cast<LuaScript* > (target));return obj;}});
	AddModuleList.insert({ "DebugCamera",[](sol::this_state s, Module* target) -> sol::object{sol::object obj = sol::make_object(s, static_cast<DebugCamera* > (target));return obj;}});
	AddModuleList.insert({ "Renderer2D",[](sol::this_state s, Module* target) -> sol::object{sol::object obj = sol::make_object(s, static_cast<Renderer2D* > (target));return obj;}});
}
inline void BindAll_GeneratedLuaModules(sol::state& lua)
{
	BindAll_AddModules();
	Generate_Engine(lua); 
	Generate_Scene(lua); 
	Generate_Asset(lua); 
	Generate_Input(lua); 
	Generate_Debug(lua); 
	Generate_Vector3(lua); 
	Generate_GameObject(lua); 
	Generate_Module(lua); 
	Generate_Transform(lua); 
	Generate_Camera(lua); 
	Generate_LuaScript(lua); 
	Generate_DebugCamera(lua); 
	Generate_Renderer2D(lua); 
	Generate_PVector3(lua); 
}
