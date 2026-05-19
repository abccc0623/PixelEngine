#include "sol.hpp"
#include "PixelEngineAPI.h" 
#include <unordered_map>
#include "Transform.h" 
#include "Renderer2D.h" 
#include "Camera.h" 
#include "Animation2D.h" 
#include "BoxCollider2D.h" 
#include "CircleCollider2D.h" 
#include "Rigidbody2D.h" 
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
	ut["AddComponent"] = &ECS::Transform::AddComponent;
	ut["GetComponent"] = &ECS::Transform::GetComponent;
	ut["HasComponent"] = &ECS::Transform::HasComponent;
	ut["BindJit"] = &ECS::Transform::BindJit;
}
inline void Generate_Renderer2D(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Renderer2D");
	ut["AddComponent"] = &ECS::Renderer2D::AddComponent;
	ut["GetComponent"] = &ECS::Renderer2D::GetComponent;
	ut["HasComponent"] = &ECS::Renderer2D::HasComponent;
	ut["BindJit"] = &ECS::Renderer2D::BindJit;
	ut["SetTexture"] = &ECS::Renderer2D::SetTexture;
}
inline void Generate_Camera(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Camera");
	ut["AddComponent"] = &ECS::Camera::AddComponent;
	ut["GetComponent"] = &ECS::Camera::GetComponent;
	ut["HasComponent"] = &ECS::Camera::HasComponent;
	ut["BindJit"] = &ECS::Camera::BindJit;
}
inline void Generate_Animation2D(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Animation2D");
	ut["Add"] = &ECS::Animation2D::Add;
	ut["Create"] = &ECS::Animation2D::Create;
	ut["Play"] = &ECS::Animation2D::Play;
	ut["Stop"] = &ECS::Animation2D::Stop;
	ut["Reset"] = &ECS::Animation2D::Reset;
}
inline void Generate_BoxCollider2D(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("BoxCollider2D");
	ut["AddComponent"] = &ECS::BoxCollider2D::AddComponent;
	ut["GetComponent"] = &ECS::BoxCollider2D::GetComponent;
	ut["HasComponent"] = &ECS::BoxCollider2D::HasComponent;
	ut["BindJit"] = &ECS::BoxCollider2D::BindJit;
}
inline void Generate_CircleCollider2D(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("CircleCollider2D");
	ut["AddComponent"] = &ECS::CircleCollider2D::AddComponent;
	ut["GetComponent"] = &ECS::CircleCollider2D::GetComponent;
	ut["HasComponent"] = &ECS::CircleCollider2D::HasComponent;
	ut["BindJit"] = &ECS::CircleCollider2D::BindJit;
}
inline void Generate_Rigidbody2D(sol::state& lua) 
{
	sol::table ut = lua.create_named_table("Rigidbody2D");
	ut["AddComponent"] = &ECS::Rigidbody2D::AddComponent;
	ut["GetComponent"] = &ECS::Rigidbody2D::GetComponent;
	ut["HasComponent"] = &ECS::Rigidbody2D::HasComponent;
	ut["BindJit"] = &ECS::Rigidbody2D::BindJit;
	ut["SetPosition"] = &ECS::Rigidbody2D::SetPosition;
	ut["SetRotation"] = &ECS::Rigidbody2D::SetRotation;
	ut["LockPosition"] = &ECS::Rigidbody2D::LockPosition;
	ut["LockRotation"] = &ECS::Rigidbody2D::LockRotation;
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
	Generate_Animation2D(lua); 
	Generate_BoxCollider2D(lua); 
	Generate_CircleCollider2D(lua); 
	Generate_Rigidbody2D(lua); 
	Generate_Entity(lua); 
}
