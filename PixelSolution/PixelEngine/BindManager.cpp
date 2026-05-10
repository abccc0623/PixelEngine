#include "pch.h"
#include "BindManager.h"

#include "Core/GameObject.h"
#include "Module/Renderer2D.h"
#include "Module/Transform.h"
#include "Module/LuaScript.h"
#include "Module/DebugCamera.h"
#include "Module/Camera.h"
#include "Module/Movement.h"
#include "Module/Physics2D.h"

#include "Type/GlobalEnum.h"
#include "Type/PVector3.h"
#include "Rect.h"

#include "PixelMetaAPI.h"
#include "PixelEngineAPI.h"
#include "Entity.h"

#include "PTransform.h"
#include "PCamera.h"
#include "PRenderer2D.h"

BindManager::BindManager()
{

}

BindManager::~BindManager()
{



}

void BindManager::Initialize()
{
	PStatic* globalCreate = CreateNewStatic("Engine");
	AddGlobalMethod(globalCreate, "CreateEntity", GeGlobalMethodInfo(&CreateEntity), MetaFlag::LUABIND);
	AddGlobalMethod(globalCreate, "DestroyEntity", GeGlobalMethodInfo(&DestroyEntity), MetaFlag::LUABIND);

	AddGlobalMethod(globalCreate, "BackgroundColor", GeGlobalMethodInfo(&BackgroundColor), MetaFlag::LUABIND);

	PStatic* globalScene = CreateNewStatic("Scene");
	AddGlobalMethod(globalScene, "ChangeScene", GeGlobalMethodInfo(&ChangeScene), MetaFlag::LUABIND);

	PStatic* globalAsset = CreateNewStatic("Asset");
	AddGlobalMethod(globalAsset, "Import", GeGlobalMethodInfo(&Import), MetaFlag::LUABIND);

	PStatic* globalInput = CreateNewStatic("Input");
	AddGlobalMethod(globalInput, "GetKey", GeGlobalMethodInfo(&GetKey), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput, "GetKeyDown", GeGlobalMethodInfo(&GetKeyDown), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput, "GetKeyUp", GeGlobalMethodInfo(&GetKeyUp), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput, "GetMousePosition_X", GeGlobalMethodInfo(&GetMousePosition_X), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput, "GetMousePosition_Y", GeGlobalMethodInfo(&GetMousePosition_Y), MetaFlag::LUABIND);

	PStatic* globaDebug = CreateNewStatic("Debug");
	AddGlobalMethod(globaDebug, "LogInfo", GeGlobalMethodInfo(&LogInfo), MetaFlag::LUABIND);
	AddGlobalMethod(globaDebug, "LogError", GeGlobalMethodInfo(&LogError), MetaFlag::LUABIND);
	AddGlobalMethod(globaDebug, "LogWarning", GeGlobalMethodInfo(&LogWarning), MetaFlag::LUABIND);

	PStatic* globalTransform = CreateNewStatic("Transform");
	AddGlobalMethod(globalTransform, "Add", GeGlobalMethodInfo(&ECS::Transform::Add), MetaFlag::LUABIND);
	AddGlobalMethod(globalTransform, "SetPosition", GeGlobalMethodInfo(&ECS::Transform::SetPosition), MetaFlag::LUABIND);
	AddGlobalMethod(globalTransform, "SetRotation", GeGlobalMethodInfo(&ECS::Transform::SetRotation), MetaFlag::LUABIND);
	AddGlobalMethod(globalTransform, "SetScale", GeGlobalMethodInfo(&ECS::Transform::SetScale), MetaFlag::LUABIND);

	PStatic* globalRenderer2D = CreateNewStatic("Renderer2D");
	AddGlobalMethod(globalRenderer2D, "Add", GeGlobalMethodInfo(&ECS::Renderer2D::Add), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "SetTexture", GeGlobalMethodInfo(&ECS::Renderer2D::SetTexture), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "SetTextureOffset", GeGlobalMethodInfo(&ECS::Renderer2D::SetTextureOffset), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "AddTextureOffset", GeGlobalMethodInfo(&ECS::Renderer2D::AddTextureOffset), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "SetTextureTiling", GeGlobalMethodInfo(&ECS::Renderer2D::SetTextureTiling), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "AddTextureTiling", GeGlobalMethodInfo(&ECS::Renderer2D::AddTextureTiling), MetaFlag::LUABIND);

	PStatic* globalCamera = CreateNewStatic("Camera");
	AddGlobalMethod(globalCamera, "Add", GeGlobalMethodInfo(&ECS::Camera::Add), MetaFlag::LUABIND);

	BindEntity();
}

void BindManager::Update()
{

}

void BindManager::Clear()
{
}

void BindManager::Release()
{

}



void BindManager::BindLuaScript()
{
	auto table = CreateNewClass("LuaScript", "Module");
	CreateClassFunction(table, []() ->void*
		{
			return new LuaScript();
		});
	DeleteClassFunction(table, []() ->void
		{
			PixelLog::Info("Delete LuaScript");
		});
	AddMethod(table, "Awake", GetMethodInfo(&LuaScript::Awake));
	AddMethod(table, "Start", GetMethodInfo(&LuaScript::Start));
	AddMethod(table, "Update", GetMethodInfo(&LuaScript::Update));

	AddMethod(table, "Register", GetMethodInfo(&LuaScript::Register), MetaFlag::LUABIND);
	AddMethod(table, "Get", GetMethodInfo(&LuaScript::Get), MetaFlag::LUABIND);
	AddMethod(table, "TriggerCustomEvent", GetMethodInfo(&LuaScript::TriggerCustomEvent), MetaFlag::LUABIND);

	AddMethod(table, "RegisterMessage", GetMethodInfo(&LuaScript::RegisterMessage), MetaFlag::LUABIND);
	AddMethod(table, "UnregisterMessage", GetMethodInfo(&LuaScript::UnregisterMessage), MetaFlag::LUABIND);
	AddMethod(table, "RegisterCustomMessage", GetMethodInfo(&LuaScript::RegisterCustomMessage), MetaFlag::LUABIND);
	AddMethod(table, "UnregisterCustomMessage", GetMethodInfo(&LuaScript::UnregisterCustomMessage), MetaFlag::LUABIND);

	AddMethod(table, "StartCoroutine", GetMethodInfo(&LuaScript::StartCoroutine), MetaFlag::LUABIND);
}

void BindManager::BindTransform()
{
	///Transform
	auto table = CreateNewClass("Transform", "Module");
	CreateClassFunction(table, []() ->void*
		{
			return new Transform();
		});
	DeleteClassFunction(table, []() ->void
		{
			PixelLog::Info("Delete Transform");
		});
	AddMember(table, "Position", GetMemberInfo(&Transform::Position), MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMember(table, "Rotation", GetMemberInfo(&Transform::Rotation), MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMember(table, "Scale", GetMemberInfo(&Transform::Scale), MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMethod(table, "Start", GetMethodInfo(&Transform::Start));
	AddMethod(table, "MatrixUpdate", GetMethodInfo(&Transform::MatrixUpdate));

	AddMethod(table, "GetLookVector", GetMethodInfo(&Transform::GetLookVector), MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMethod(table, "GetRightVector", GetMethodInfo(&Transform::GetRightVector), MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMethod(table, "GetUpVector", GetMethodInfo(&Transform::GetUpVector), MetaFlag::LUABIND | MetaFlag::SAVE);
}

//void BindManager::BindRenderer2D()
//{
//	///Renderer2D 
//	auto table = CreateNewClass("Renderer2D", "Module");
//	CreateClassFunction(table, []() ->void*
//		{
//			return new Renderer2D();
//		});
//	DeleteClassFunction(table, []() ->void
//		{
//			PixelLog::Info("Delete Renderer2D");
//		});
//	AddMethod(table, "LastUpdate", GetMethodInfo(&Renderer2D::LastUpdate));
//	AddMethod(table, "SetTexture", GetMethodInfo(&Renderer2D::SetTexture), MetaFlag::LUABIND);
//	AddMethod(table, "SetMaterial", GetMethodInfo(&Renderer2D::SetMaterial), MetaFlag::LUABIND);
//
//	AddMethod(table, "CreateAnimation", GetMethodInfo(&Renderer2D::CreateAnimation), MetaFlag::LUABIND);
//	AddMethod(table, "PlayAnimation", GetMethodInfo(&Renderer2D::PlayAnimation), MetaFlag::LUABIND);
//
//	AddMethod(table, "SetTextureOffset", GetMethodInfo(&Renderer2D::SetTextureOffset), MetaFlag::LUABIND);
//	AddMethod(table, "AddTextureOffset", GetMethodInfo(&Renderer2D::AddTextureOffset), MetaFlag::LUABIND);
//	AddMethod(table, "SetTextureTiling", GetMethodInfo(&Renderer2D::SetTextureTiling), MetaFlag::LUABIND);
//	AddMethod(table, "AddTextureTiling", GetMethodInfo(&Renderer2D::AddTextureTiling), MetaFlag::LUABIND);
//}

void BindManager::BindDebugCamera()
{
	///DebugCamera
	auto table = CreateNewClass("DebugCamera", "Module");
	CreateClassFunction(table, []() ->void*
		{
			return new DebugCamera();
		});
	DeleteClassFunction(table, []() ->void
		{
			PixelLog::Info("Delete DebugCamera");
		});
	AddMethod(table, "Start", GetMethodInfo(&DebugCamera::Start));
	AddMethod(table, "Update", GetMethodInfo(&DebugCamera::Update));
	AddMethod(table, "LastUpdate", GetMethodInfo(&DebugCamera::LastUpdate));
}

void BindManager::BindCamera()
{
	///Camera
	auto table = CreateNewClass("Camera", "Module");
	CreateClassFunction(table, []() ->void*
		{
			return new Camera();
		});
	DeleteClassFunction(table, []() ->void
		{
			PixelLog::Info("Delete Camera");
		});
	AddMethod(table, "Start", GetMethodInfo(&Camera::Start));
	AddMethod(table, "LastUpdate", GetMethodInfo(&Camera::LastUpdate));
	AddMethod(table, "OrthographicProjection", GetMethodInfo(&Camera::OrthographicProjection));
	AddMethod(table, "PerspectiveProjection", GetMethodInfo(&Camera::PerspectiveProjection));
}

void BindManager::BindMovement()
{
	auto table = CreateNewClass("Movement", "Module");
	CreateClassFunction(table, []() ->void*
		{
			return new Movement();
		});
	DeleteClassFunction(table, []() ->void
		{
			PixelLog::Info("Delete Movement");
		});
	AddMember(table, "stopDistance", GetMemberInfo(&Movement::stopDistance));
	AddMember(table, "speed", GetMemberInfo(&Movement::speed));
	AddMethod(table, "MoveToTarget", GetMethodInfo(&Movement::MoveToTarget), MetaFlag::LUABIND);
	AddMethod(table, "MoveToPosition", GetMethodInfo(&Movement::MoveToPosition), MetaFlag::LUABIND);
	AddMethod(table, "StopMove", GetMethodInfo(&Movement::StopMove), MetaFlag::LUABIND);
	AddMethod(table, "AddCompleteCallBack", GetMethodInfo(&Movement::AddCompleteCallBack), MetaFlag::LUABIND);
	AddMethod(table, "AddStartedCallBack", GetMethodInfo(&Movement::AddStartedCallBack), MetaFlag::LUABIND);
	AddMethod(table, "AddDirectionCallBack", GetMethodInfo(&Movement::AddDirectionCallBack), MetaFlag::LUABIND);
	AddMethod(table, "Update", GetMethodInfo(&Movement::Update));
	AddMethod(table, "Start", GetMethodInfo(&Movement::Start));
}

void BindManager::BindGameObject()
{
	auto table = CreateNewClass("GameObject");
	AddMethod(table, "AddModule", GetMethodInfo(&GameObject::AddModule), MetaFlag::LUABIND);
	AddMethod(table, "GetModule", GetMethodInfo(&GameObject::GetModule), MetaFlag::LUABIND);
}

void BindManager::BindPVector3()
{
	auto table = CreateNewClass("PVector3");
	AddMember(table, "X", GetMemberInfo(&PVector3::X));
	AddMember(table, "Y", GetMemberInfo(&PVector3::Y));
	AddMember(table, "Z", GetMemberInfo(&PVector3::Z));
	AddMethod(table, "Create", GetMethodInfo(&PVector3::Create), MetaFlag::LUABIND);
	AddMethod(table, "Normalize", GetMethodInfo(&PVector3::Normalize), MetaFlag::LUABIND);
}

void BindManager::BindEnum()
{
	PEnum* globalEnum = nullptr;
	globalEnum = CreateNewEnum("EventType");
	AddEnum(globalEnum, "KeyUp");
	AddEnum(globalEnum, "KeyDown");
	AddEnum(globalEnum, "CollisionIn");
	AddEnum(globalEnum, "CollisionOut");

	globalEnum = CreateNewEnum("ColliderMotionType");
	AddEnum(globalEnum, "Static");
	AddEnum(globalEnum, "Kinematic");
	AddEnum(globalEnum, "Dynamic");

	globalEnum = CreateNewEnum("ColliderType");
	AddEnum(globalEnum, "Box2D");
	AddEnum(globalEnum, "Circle2D");

}

void BindManager::BindPhysics2D()
{
	auto table = CreateNewClass("Physics2D", "Module");
	CreateClassFunction(table, []() ->void*
		{
			return new Physics2D();
		});
	DeleteClassFunction(table, []() ->void
		{
			PixelLog::Info("Delete Physics2D");
		});
	AddMethod(table, "Awake", GetMethodInfo(&Physics2D::Awake));
	AddMethod(table, "LastUpdate", GetMethodInfo(&Physics2D::LastUpdate));
	AddMethod(table, "PhysicsUpdate", GetMethodInfo(&Physics2D::PhysicsUpdate));

	AddMethod(table, "SetCollider", GetMethodInfo(&Physics2D::SetCollider), MetaFlag::LUABIND);
	AddMethod(table, "SetRigidbody", GetMethodInfo(&Physics2D::SetRigidbody), MetaFlag::LUABIND);

	AddMethod(table, "SetVelocity", GetMethodInfo(&Physics2D::SetVelocity), MetaFlag::LUABIND);
	AddMethod(table, "SetVelocityX", GetMethodInfo(&Physics2D::SetVelocityX), MetaFlag::LUABIND);
	AddMethod(table, "SetVelocityY", GetMethodInfo(&Physics2D::SetVelocityY), MetaFlag::LUABIND);
	AddMethod(table, "SetPosition", GetMethodInfo(&Physics2D::SetPosition), MetaFlag::LUABIND);
	AddMethod(table, "SetRotation", GetMethodInfo(&Physics2D::SetRotation), MetaFlag::LUABIND);
	AddMethod(table, "SetActive", GetMethodInfo(&Physics2D::SetActive), MetaFlag::LUABIND);

	AddMethod(table, "AddImpulse", GetMethodInfo(&Physics2D::AddImpulse), MetaFlag::LUABIND);
	AddMethod(table, "AddForce", GetMethodInfo(&Physics2D::AddForce), MetaFlag::LUABIND);
}

void BindManager::BindEntity()
{
	auto table = CreateNewClass("Entity");
	AddMember(table, "Active", GetMemberInfo(&ECS::Entity::Active), MetaFlag::LUABIND);
}




