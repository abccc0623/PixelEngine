#include "pch.h"
#include "BindManager.h"

#include "Core/GameObject.h"
#include "Module/Renderer2D.h"
#include "Module/Transform.h"
#include "Module/LuaScript.h"
#include "Module/DebugCamera.h"
#include "Module/Camera.h"

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

void Test() 
{

}
void BindManager::Initialize()
{
	PNamespace* globalCreate = CreateNewNamepace("Engine");
	AddGlobalMethod(globalCreate,"CreateGameObject", GeGlobalMethodInfo(&CreateGameObject), MetaFlag::LUABIND);

	PNamespace* globalScene = CreateNewNamepace("Scene");
	AddGlobalMethod(globalScene,"ChangeScene",GeGlobalMethodInfo(&ChangeScene), MetaFlag::LUABIND);

	PNamespace* globalAsset = CreateNewNamepace("Asset");
	AddGlobalMethod(globalAsset,"Import",GeGlobalMethodInfo(&Import), MetaFlag::LUABIND);

	PNamespace* globalInput = CreateNewNamepace("Input");
	AddGlobalMethod(globalInput,"GetKey",GeGlobalMethodInfo(&GetKey), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput,"GetKeyDown",GeGlobalMethodInfo(&GetKeyDown), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput,"GetKeyUp",GeGlobalMethodInfo(&GetKeyUp), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput,"GetMousePosition_X",GeGlobalMethodInfo(&GetMousePosition_X), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput,"GetMousePosition_Y",GeGlobalMethodInfo(&GetMousePosition_Y), MetaFlag::LUABIND);

	
	PClass* table = nullptr;
	table = CreateNewClass("PVector3");
	AddMember(table, "X", GetMemberInfo(&PVector3::X));
	AddMember(table, "Y", GetMemberInfo(&PVector3::Y));
	AddMember(table, "Z", GetMemberInfo(&PVector3::Z));
	
	//기본 게임 오브젝트 상속관계
	table = CreateNewClass("PixelObject");
	table = CreateNewClass("GameObject", "PixelObject");

	//기본 모듈 상속관계
	table = CreateNewClass("Module",		"PixelObject");
	table = CreateNewClass("Transform",		"Module");
	table = CreateNewClass("Camera",		"Module");
	table = CreateNewClass("LuaScript",		"Module");
	//table = CreateNewClass("Collision2D",	"Module");
	table = CreateNewClass("DebugCamera",	"Module");
	table = CreateNewClass("LuaScript",		"Module");
	//table = CreateNewClass("Renderer",		"Module");
	table = CreateNewClass("Renderer2D",	"Module");
	
	BindLuaScript();
	BindGameObject();
	BindTransform();
	BindRenderer2D();
	BindDebugCamera();
	BindCamera();
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
	auto table = GetClass("LuaScript");
	CreateClassFunction(table, []() ->void*
		{
			return new LuaScript();
		});
	DeleteClassFunction(table, []() ->void
		{
			Log::Info("Delete LuaScript");
		});
	AddMethod(table, "Awake", GetMethodInfo(&LuaScript::Awake));
	AddMethod(table, "Start", GetMethodInfo(&LuaScript::Start));
	AddMethod(table, "Update", GetMethodInfo(&LuaScript::Update));
	AddMethod(table, "Register", GetMethodInfo(&LuaScript::Register), MetaFlag::LUABIND | MetaFlag::SAVE);
}

void BindManager::BindTransform()
{
	///Transform
	auto table = GetClass("Transform");
	CreateClassFunction(table, []() ->void*
		{
			return new Transform();
		});
	DeleteClassFunction(table, []() ->void
		{
			Log::Info("Delete Transform");
		});
	AddMember(table, "Position", GetMemberInfo(&Transform::Position), MetaFlag::LUABIND| MetaFlag::SAVE);
	AddMember(table, "Rotation", GetMemberInfo(&Transform::Rotation), MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMember(table, "Scale", GetMemberInfo(&Transform::Scale),MetaFlag::LUABIND | MetaFlag::SAVE);
	AddMethod(table, "Start", GetMethodInfo(&Transform::Start));
	AddMethod(table, "MatrixUpdate", GetMethodInfo(&Transform::MatrixUpdate));
}

void BindManager::BindRenderer2D()
{
	///Renderer2D 
	auto table = GetClass("Renderer2D");
	CreateClassFunction(table, []() ->void*
		{
			return new Renderer2D();
		});
	DeleteClassFunction(table, []() ->void
		{
			Log::Info("Delete Renderer2D");
		});
	AddMethod(table, "LastUpdate", GetMethodInfo(&Renderer2D::LastUpdate));
	AddMethod(table, "SetTexture", GetMethodInfo(&Renderer2D::SetTexture), MetaFlag::LUABIND);
}

void BindManager::BindDebugCamera()
{
	///DebugCamera
	auto table = GetClass("DebugCamera");
	CreateClassFunction(table, []() ->void*
		{
			return new DebugCamera();
		});
	DeleteClassFunction(table, []() ->void
		{
			Log::Info("Delete DebugCamera");
		});
	AddMethod(table, "Start", GetMethodInfo(&DebugCamera::Start));
	AddMethod(table, "Update", GetMethodInfo(&DebugCamera::Update));
	AddMethod(table, "LastUpdate", GetMethodInfo(&DebugCamera::LastUpdate));
}

void BindManager::BindCamera()
{
	///Camera
	auto table = GetClass("Camera");
	CreateClassFunction(table, []() ->void*
		{
			return new Camera();
		});
	DeleteClassFunction(table, []() ->void
		{
			Log::Info("Delete Camera");
		});
	AddMethod(table, "Start", GetMethodInfo(&Camera::Start));
	AddMethod(table, "LastUpdate", GetMethodInfo(&Camera::LastUpdate));
}

void BindManager::BindGameObject()
{
	auto table = GetClass("GameObject");
	AddMethod(table, "AddModule", GetMethodInfo(&GameObject::AddModule), MetaFlag::LUABIND);
	AddMethod(table, "GetModule", GetMethodInfo(&GameObject::GetModule), MetaFlag::LUABIND);
}




