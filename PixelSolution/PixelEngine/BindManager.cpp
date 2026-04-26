#include "pch.h"
#include "BindManager.h"

#include "Core/GameObject.h"
#include "Module/Renderer2D.h"
#include "Module/Transform.h"
#include "Module/LuaScript.h"
#include "Module/DebugCamera.h"
#include "Module/Camera.h"
#include "Module/Movement.h"
#include "Module/BoxCollider2D.h"

#include "Type/GlobalEnum.h"
#include "Type/PVector3.h"
#include "Rect.h"

#include "PixelMetaAPI.h"
#include "PixelEngineAPI.h"


BindManager::BindManager()
{

}

BindManager::~BindManager()
{



}

void BindManager::Initialize()
{
	PStatic* globalCreate = CreateNewStatic("Engine");
	AddGlobalMethod(globalCreate,"CreateGameObject", GeGlobalMethodInfo(&CreateGameObject), MetaFlag::LUABIND);
	AddGlobalMethod(globalCreate,"FindGameObject", GeGlobalMethodInfo(&FindGameObject), MetaFlag::LUABIND);
	AddGlobalMethod(globalCreate,"BackgroundColor", GeGlobalMethodInfo(&BackgroundColor), MetaFlag::LUABIND);
	
	PStatic* globalScene = CreateNewStatic("Scene");
	AddGlobalMethod(globalScene,"ChangeScene",GeGlobalMethodInfo(&ChangeScene), MetaFlag::LUABIND);
	
	PStatic* globalAsset = CreateNewStatic("Asset");
	AddGlobalMethod(globalAsset,"Import",GeGlobalMethodInfo(&Import), MetaFlag::LUABIND);
	
	PStatic* globalInput = CreateNewStatic("Input");
	AddGlobalMethod(globalInput,"GetKey",GeGlobalMethodInfo(&GetKey), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput,"GetKeyDown",GeGlobalMethodInfo(&GetKeyDown), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput,"GetKeyUp",GeGlobalMethodInfo(&GetKeyUp), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput,"GetMousePosition_X",GeGlobalMethodInfo(&GetMousePosition_X), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput,"GetMousePosition_Y",GeGlobalMethodInfo(&GetMousePosition_Y), MetaFlag::LUABIND);
	
	PStatic* globaDebug = CreateNewStatic("Debug");
	AddGlobalMethod(globaDebug, "LogInfo", GeGlobalMethodInfo(&LogInfo), MetaFlag::LUABIND);
	AddGlobalMethod(globaDebug, "LogError", GeGlobalMethodInfo(&LogError), MetaFlag::LUABIND);
	AddGlobalMethod(globaDebug, "LogWarning", GeGlobalMethodInfo(&LogWarning), MetaFlag::LUABIND);
	
	PStatic* globaVector3 = CreateNewStatic("Vector3");
	AddGlobalMethod(globaVector3, "Lerp",		GeGlobalMethodInfo(&Lerp),		MetaFlag::LUABIND);
	AddGlobalMethod(globaVector3, "Distance",	GeGlobalMethodInfo(&Distance),	MetaFlag::LUABIND);
	CreateNewClass("Module");
	
	BindPVector3();
	BindLuaScript();
	BindGameObject();
	BindTransform();
	BindMovement();
	BindRenderer2D();
	BindDebugCamera();
	BindCamera();
	BindBoxCollider2D();
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
	AddMethod(table, "Awake",					GetMethodInfo(&LuaScript::Awake));
	AddMethod(table, "Start",					GetMethodInfo(&LuaScript::Start));
	AddMethod(table, "Update",					GetMethodInfo(&LuaScript::Update));

	AddMethod(table, "Register",				GetMethodInfo(&LuaScript::Register),				MetaFlag::LUABIND);
	AddMethod(table, "Get",						GetMethodInfo(&LuaScript::Get),						MetaFlag::LUABIND);
	AddMethod(table, "TriggerCustomEvent",		GetMethodInfo(&LuaScript::TriggerCustomEvent),		MetaFlag::LUABIND);
	
	AddMethod(table, "RegisterMessage",			GetMethodInfo(&LuaScript::RegisterMessage),			MetaFlag::LUABIND);
	AddMethod(table, "UnregisterMessage",		GetMethodInfo(&LuaScript::UnregisterMessage),		MetaFlag::LUABIND);
	AddMethod(table, "RegisterCustomMessage",	GetMethodInfo(&LuaScript::RegisterCustomMessage),	MetaFlag::LUABIND);
	AddMethod(table, "UnregisterCustomMessage", GetMethodInfo(&LuaScript::UnregisterCustomMessage), MetaFlag::LUABIND);

	AddMethod(table, "StartCoroutine",			GetMethodInfo(&LuaScript::StartCoroutine),			MetaFlag::LUABIND);
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
	AddMember(table, "Position", GetMemberInfo(&Transform::Position), MetaFlag::LUABIND| MetaFlag::SAVE);
	AddMember(table, "Rotation", GetMemberInfo(&Transform::Rotation), MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMember(table, "Scale", GetMemberInfo(&Transform::Scale),MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMethod(table, "Start", GetMethodInfo(&Transform::Start));
	AddMethod(table, "MatrixUpdate", GetMethodInfo(&Transform::MatrixUpdate));
	
	AddMethod(table, "GetLookVector", GetMethodInfo(&Transform::GetLookVector), MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMethod(table, "GetRightVector", GetMethodInfo(&Transform::GetRightVector), MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMethod(table, "GetUpVector", GetMethodInfo(&Transform::GetUpVector), MetaFlag::LUABIND | MetaFlag::SAVE);
}

void BindManager::BindRenderer2D()
{
	///Renderer2D 
	auto table = CreateNewClass("Renderer2D", "Module");
	CreateClassFunction(table, []() ->void*
		{
			return new Renderer2D();
		});
	DeleteClassFunction(table, []() ->void
		{
			PixelLog::Info("Delete Renderer2D");
		});
	AddMethod(table, "LastUpdate", GetMethodInfo(&Renderer2D::LastUpdate));
	AddMethod(table, "SetTexture", GetMethodInfo(&Renderer2D::SetTexture), MetaFlag::LUABIND);
	AddMethod(table, "SetMaterial", GetMethodInfo(&Renderer2D::SetMaterial), MetaFlag::LUABIND);

	AddMethod(table, "CreateAnimation", GetMethodInfo(&Renderer2D::CreateAnimation), MetaFlag::LUABIND);
	AddMethod(table, "PlayAnimation", GetMethodInfo(&Renderer2D::PlayAnimation), MetaFlag::LUABIND);

	AddMethod(table, "SetTextureOffset", GetMethodInfo(&Renderer2D::SetTextureOffset), MetaFlag::LUABIND);
	AddMethod(table, "AddTextureOffset", GetMethodInfo(&Renderer2D::AddTextureOffset), MetaFlag::LUABIND);
	AddMethod(table, "SetTextureTiling", GetMethodInfo(&Renderer2D::SetTextureTiling), MetaFlag::LUABIND);
	AddMethod(table, "AddTextureTiling", GetMethodInfo(&Renderer2D::AddTextureTiling), MetaFlag::LUABIND);
}

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
	AddMember(table, "stopDistance",			GetMemberInfo(&Movement::stopDistance));
	AddMember(table, "speed",					GetMemberInfo(&Movement::speed));
	AddMethod(table, "MoveToTarget",			GetMethodInfo(&Movement::MoveToTarget),			MetaFlag::LUABIND);
	AddMethod(table, "MoveToPosition",			GetMethodInfo(&Movement::MoveToPosition),		MetaFlag::LUABIND);
	AddMethod(table, "StopMove",				GetMethodInfo(&Movement::StopMove),				MetaFlag::LUABIND);
	AddMethod(table, "AddCompleteCallBack",		GetMethodInfo(&Movement::AddCompleteCallBack),	MetaFlag::LUABIND);
	AddMethod(table, "AddStartedCallBack",		GetMethodInfo(&Movement::AddStartedCallBack),	MetaFlag::LUABIND);
	AddMethod(table, "AddDirectionCallBack",	GetMethodInfo(&Movement::AddDirectionCallBack),	MetaFlag::LUABIND);
	AddMethod(table, "Update",					GetMethodInfo(&Movement::Update));
}

void BindManager::BindGameObject()
{
	auto table = CreateNewClass("GameObject");
	AddMethod(table, "AddModule", GetMethodInfo(&GameObject::AddModule), MetaFlag::LUABIND);
	AddMethod(table, "GetModule", GetMethodInfo(&GameObject::GetModule), MetaFlag::LUABIND);
	AddMethod(table, "GetTransform", GetMethodInfo(&GameObject::GetTransform), MetaFlag::LUABIND);
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
	PEnum* globalEnum = CreateNewEnum("EventType");
	AddEnum(globalEnum, "KeyUp");
	AddEnum(globalEnum, "KeyDown");
}

void BindManager::BindBoxCollider2D()
{
	auto table = CreateNewClass("BoxCollider2D", "Module");
	CreateClassFunction(table, []() ->void*
		{
			return new BoxCollider2D();
		});
	DeleteClassFunction(table, []() ->void
		{
			PixelLog::Info("Delete BoxCollider2D");
		});
	AddMethod(table, "Awake", GetMethodInfo(&BoxCollider2D::Awake));
	AddMethod(table, "LastUpdate", GetMethodInfo(&BoxCollider2D::LastUpdate));
	AddMethod(table, "PhysicsUpdate", GetMethodInfo(&BoxCollider2D::PhysicsUpdate));

	AddMethod(table, "SetCenter",		GetMethodInfo(&BoxCollider2D::SetCenter),		MetaFlag::LUABIND);
	AddMethod(table, "SetOffset",		GetMethodInfo(&BoxCollider2D::SetOffset),		MetaFlag::LUABIND);
	AddMethod(table, "SetGravity",		GetMethodInfo(&BoxCollider2D::SetGravity),		MetaFlag::LUABIND);
	AddMethod(table, "SetCreateActive", GetMethodInfo(&BoxCollider2D::SetCreateActive), MetaFlag::LUABIND);
	AddMethod(table, "SetPhysType",		GetMethodInfo(&BoxCollider2D::SetPhysType),		MetaFlag::LUABIND);
	AddMethod(table, "CreatePhys",		GetMethodInfo(&BoxCollider2D::CreatePhys),		MetaFlag::LUABIND);
}




