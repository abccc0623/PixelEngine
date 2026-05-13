#include "sol.hpp"
#include "PixelEngineAPI.h" 
#include <unordered_map>
#include "Transform.h" 
#include "Renderer2D.h" 
#include "Camera.h" 
#include "Animation2D.h" 
#include "Collider2D.h" 
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
inline void Generate_Animation2D(sol::state& lua)
{
	sol::table ut = lua.create_named_table("Animation2D");
	ut["Add"] = &ECS::Animation2D::Add;
	ut["Create"] = &ECS::Animation2D::Create;
	ut["Play"] = &ECS::Animation2D::Play;
	ut["Stop"] = &ECS::Animation2D::Stop;
	ut["Reset"] = &ECS::Animation2D::Reset;
}
inline void Generate_Collider2D(sol::state& lua)
{
	sol::table ut = lua.create_named_table("Collider2D");
	ut["Add"] = &ECS::Collider2D::Add;
	ut["SetBoxOffset"] = &ECS::Collider2D::SetBoxOffset;
	ut["SetCircleOffset"] = &ECS::Collider2D::SetCircleOffset;
	ut["SetCenter"] = &ECS::Collider2D::SetCenter;
}
inline void Generate_Rigidbody2D(sol::state& lua)
{
	sol::table ut = lua.create_named_table("Rigidbody2D");
	ut["Add"] = &ECS::Rigidbody2D::Add;
	ut["SetMotionType"] = &ECS::Rigidbody2D::SetMotionType;
	ut["SetPosition"] = &ECS::Rigidbody2D::SetPosition;
	ut["SetRotation"] = &ECS::Rigidbody2D::SetRotation;
	ut["SetKinematic"] = &ECS::Rigidbody2D::SetKinematic;
	ut["SetAutoSleep"] = &ECS::Rigidbody2D::SetAutoSleep;
	ut["SetSensor"] = &ECS::Rigidbody2D::SetSensor;
	ut["SetGravity"] = &ECS::Rigidbody2D::SetGravity;
	ut["SetRestitution"] = &ECS::Rigidbody2D::SetRestitution;
	ut["SetFriction"] = &ECS::Rigidbody2D::SetFriction;
	ut["SetLinearDamping"] = &ECS::Rigidbody2D::SetLinearDamping;
	ut["SetPositionLock"] = &ECS::Rigidbody2D::SetPositionLock;
	ut["SetRotationLock"] = &ECS::Rigidbody2D::SetRotationLock;
}
inline void Generate_Entity(sol::state& lua)
{
	sol::usertype<Entity> ut = lua.new_usertype<Entity>("Entity");
	ut["Active"] = &Entity::Active;
}
inline void Generate_MotionType(sol::state& lua)
{
	lua.new_enum<MotionType>("MotionType", {
	{ "Static", MotionType::Static },
	{ "Kinematic", MotionType::Kinematic },
	{ "Dynamic", MotionType::Dynamic },
		});
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
	Generate_Collider2D(lua);
	Generate_Rigidbody2D(lua);
	Generate_Entity(lua);
	Generate_MotionType(lua);
}
