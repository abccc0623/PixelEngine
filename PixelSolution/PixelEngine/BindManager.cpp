#include "pch.h"
#include "BindManager.h"

#include "Module/Movement.h"
#include "Module/Physics2D.h"

#include "Type/GlobalEnum.h"
#include "Type/PVector3.h"
#include "Rect.h"

#include "PixelMetaAPI.h"
#include "PixelEngineAPI.h"
#include "Entity.h"

#include "Transform.h"
#include "Camera.h"
#include "Animation2D.h"
#include "Renderer2D.h"
#include "Collider2D.h"
#include "BoxCollider2D.h"
#include "Rigidbody2D.h"

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
	AddGlobalMethod(globalTransform, "AddComponent", GeGlobalMethodInfo(&ECS::Transform::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalTransform, "GetComponent", GeGlobalMethodInfo(&ECS::Transform::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalTransform, "HasComponent", GeGlobalMethodInfo(&ECS::Transform::HasComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalTransform, "BindJit", GeGlobalMethodInfo(&ECS::Transform::BindJit), MetaFlag::LUABIND);


	PStatic* globalRenderer2D = CreateNewStatic("Renderer2D");
	AddGlobalMethod(globalRenderer2D, "AddComponent", GeGlobalMethodInfo(&ECS::Renderer2D::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "GetComponent", GeGlobalMethodInfo(&ECS::Renderer2D::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "HasComponent", GeGlobalMethodInfo(&ECS::Renderer2D::HasComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "BindJit", GeGlobalMethodInfo(&ECS::Renderer2D::BindJit), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "SetTexture", GeGlobalMethodInfo(&ECS::Renderer2D::SetTexture), MetaFlag::LUABIND);


	PStatic* globalCamera = CreateNewStatic("Camera");
	AddGlobalMethod(globalCamera, "AddComponent", GeGlobalMethodInfo(&ECS::Camera::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalCamera, "GetComponent", GeGlobalMethodInfo(&ECS::Camera::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalCamera, "HasComponent", GeGlobalMethodInfo(&ECS::Camera::HasComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalCamera, "BindJit", GeGlobalMethodInfo(&ECS::Camera::BindJit), MetaFlag::LUABIND);

	PStatic* globalAnimation2D = CreateNewStatic("Animation2D");
	AddGlobalMethod(globalAnimation2D, "Add", GeGlobalMethodInfo(&ECS::Animation2D::Add), MetaFlag::LUABIND);
	AddGlobalMethod(globalAnimation2D, "Create", GeGlobalMethodInfo(&ECS::Animation2D::Create), MetaFlag::LUABIND);
	AddGlobalMethod(globalAnimation2D, "Play", GeGlobalMethodInfo(&ECS::Animation2D::Play), MetaFlag::LUABIND);
	AddGlobalMethod(globalAnimation2D, "Stop", GeGlobalMethodInfo(&ECS::Animation2D::Stop), MetaFlag::LUABIND);
	AddGlobalMethod(globalAnimation2D, "Reset", GeGlobalMethodInfo(&ECS::Animation2D::Reset), MetaFlag::LUABIND);

	PStatic* globalBoxCollider2D = CreateNewStatic("BoxCollider2D");
	AddGlobalMethod(globalBoxCollider2D, "AddComponent", GeGlobalMethodInfo(&ECS::BoxCollider2D::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalBoxCollider2D, "GetComponent", GeGlobalMethodInfo(&ECS::BoxCollider2D::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalBoxCollider2D, "HasComponent", GeGlobalMethodInfo(&ECS::BoxCollider2D::HasComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalBoxCollider2D, "BindJit", GeGlobalMethodInfo(&ECS::BoxCollider2D::BindJit), MetaFlag::LUABIND);

	PStatic* globalRigidbody2D = CreateNewStatic("Rigidbody2D");
	AddGlobalMethod(globalRigidbody2D, "AddComponent", GeGlobalMethodInfo(&ECS::Rigidbody2D::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRigidbody2D, "GetComponent", GeGlobalMethodInfo(&ECS::Rigidbody2D::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRigidbody2D, "HasComponent", GeGlobalMethodInfo(&ECS::Rigidbody2D::HasComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRigidbody2D, "BindJit", GeGlobalMethodInfo(&ECS::Rigidbody2D::BindJit), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "Add", GeGlobalMethodInfo(&ECS::Rigidbody2D::Add), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetMotionType", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetMotionType), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetPosition", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetPosition), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetRotation", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetRotation), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetKinematic", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetKinematic), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetAutoSleep", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetAutoSleep), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetSensor", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetSensor), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetGravity", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetGravity), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetRestitution", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetRestitution), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetFriction", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetFriction), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetLinearDamping", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetLinearDamping), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetPositionLock", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetPositionLock), MetaFlag::LUABIND);
	//AddGlobalMethod(globalRigidbody2D, "SetRotationLock", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetRotationLock), MetaFlag::LUABIND);

	BindEntity();
	BindEnum();
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
	//AddMethod(table, "MoveToTarget", GetMethodInfo(&Movement::MoveToTarget), MetaFlag::LUABIND);
	AddMethod(table, "MoveToPosition", GetMethodInfo(&Movement::MoveToPosition), MetaFlag::LUABIND);
	AddMethod(table, "StopMove", GetMethodInfo(&Movement::StopMove), MetaFlag::LUABIND);
	AddMethod(table, "AddCompleteCallBack", GetMethodInfo(&Movement::AddCompleteCallBack), MetaFlag::LUABIND);
	AddMethod(table, "AddStartedCallBack", GetMethodInfo(&Movement::AddStartedCallBack), MetaFlag::LUABIND);
	AddMethod(table, "AddDirectionCallBack", GetMethodInfo(&Movement::AddDirectionCallBack), MetaFlag::LUABIND);
	AddMethod(table, "Update", GetMethodInfo(&Movement::Update));
	AddMethod(table, "Start", GetMethodInfo(&Movement::Start));
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
	//globalEnum = CreateNewEnum("EventType");
	//AddEnum(globalEnum, "KeyUp");
	//AddEnum(globalEnum, "KeyDown");
	//AddEnum(globalEnum, "CollisionIn");
	//AddEnum(globalEnum, "CollisionOut");

	//globalEnum = CreateNewEnum("MotionType");
	//AddEnum(globalEnum, "Static");
	//AddEnum(globalEnum, "Kinematic");
	//AddEnum(globalEnum, "Dynamic");

	//globalEnum = CreateNewEnum("ColliderType");
	//AddEnum(globalEnum, "Box2D");
	//AddEnum(globalEnum, "Circle2D");

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




