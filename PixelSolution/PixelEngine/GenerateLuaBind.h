#include "sol.hpp"
#include "PixelEngineAPI.h" 
#include <unordered_map>
#include "Transform.h" 
#include "Renderer2D.h" 
#include "Camera.h" 
#include "Entity.h" 
using namespace ECS;
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
	ut["Add"] = &ECS::Transform::Add;
	ut["SetPosition"] = &ECS::Transform::SetPosition;
	ut["SetRotation"] = &ECS::Transform::SetRotation;
	ut["SetScale"] = &ECS::Transform::SetScale;
	ut["AddPosition"] = &ECS::Transform::AddPosition;
	ut["AddRotation"] = &ECS::Transform::AddRotation;
	ut["AddScale"] = &ECS::Transform::AddScale;
}
inline void Generate_Renderer2D(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Renderer2D");
	ut["Add"] = &ECS::Renderer2D::Add;
	ut["SetTexture"] = &ECS::Renderer2D::SetTexture;
	ut["SetTextureOffset"] = &ECS::Renderer2D::SetTextureOffset;
	ut["AddTextureOffset"] = &ECS::Renderer2D::AddTextureOffset;
	ut["SetTextureTiling"] = &ECS::Renderer2D::SetTextureTiling;
	ut["AddTextureTiling"] = &ECS::Renderer2D::AddTextureTiling;
}
inline void Generate_Camera(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Camera");
	ut["Add"] = &ECS::Camera::Add;
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
	Generate_Renderer2D(lua); 
	Generate_Camera(lua); 
	Generate_Entity(lua); 
}
