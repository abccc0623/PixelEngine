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

BindManager::BindManager()
{

}

BindManager::~BindManager()
{

}

void BindManager::Initialize()
{
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
	table = CreateNewClass("Collision2D",	"Module");
	table = CreateNewClass("DebugCamera",	"Module");
	table = CreateNewClass("LuaScript",		"Module");
	table = CreateNewClass("Renderer",		"Module");
	table = CreateNewClass("Renderer2D",	"Module");
	
	BindTransform();
	BindRenderer2D();
	BindDebugCamera();
	BindCamera();
}

void BindManager::Update()
{

}

void BindManager::ReleaseShared()
{

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
	AddMember(table, "Position", GetMemberInfo(&Transform::Position));
	AddMember(table, "Rotation", GetMemberInfo(&Transform::Rotation));
	AddMember(table, "Scale", GetMemberInfo(&Transform::Scale));
	AddMethod(table, "Start", GeMethodInfo(&Transform::Start));
	AddMethod(table, "MatrixUpdate", GeMethodInfo(&Transform::MatrixUpdate));
}

void BindManager::BindRenderer2D()
{
	///Renderer2D 
	auto table = GetClass("Renderer2D");
	CreateClassFunction(table, []() ->void*
		{
			Log::Info("Create Renderer2D");
			return new Renderer2D();
		});
	DeleteClassFunction(table, []() ->void
		{
			Log::Info("Delete Renderer2D");
		});
	AddMethod(table, "LastUpdate", GeMethodInfo(&Renderer2D::LastUpdate));
	AddMethod(table, "SetTexture", GeMethodInfo(&Renderer2D::SetTexture));
}

void BindManager::BindDebugCamera()
{
	///DebugCamera
	auto table = GetClass("DebugCamera");
	CreateClassFunction(table, []() ->void*
		{
			Log::Info("Create DebugCamera");
			return new DebugCamera();
		});
	DeleteClassFunction(table, []() ->void
		{
			Log::Info("Delete DebugCamera");
		});
	AddMethod(table, "Start", GeMethodInfo(&DebugCamera::Start));
	AddMethod(table, "Update", GeMethodInfo(&DebugCamera::Update));
	AddMethod(table, "LastUpdate", GeMethodInfo(&DebugCamera::LastUpdate));
}

void BindManager::BindCamera()
{
	///Camera
	auto table = GetClass("Camera");
	CreateClassFunction(table, []() ->void*
		{
			Log::Info("Create Camera");
			return new Camera();
		});
	DeleteClassFunction(table, []() ->void
		{
			Log::Info("Delete Camera");
		});
	AddMethod(table, "Start", GeMethodInfo(&Camera::Start));
	AddMethod(table, "LastUpdate", GeMethodInfo(&Camera::LastUpdate));
}




