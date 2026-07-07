#include "pch.h"
#include "BindManager.h"
#include "Type/GlobalEnum.h"
#include "PVector2.h"
#include "PVector3.h"

#include "PixelMetaAPI.h"
#include "EngineMetaFlag.h"
#include "PixelEngineAPI.h"
#include "EntityObject.h"

#include "Transform.h"
#include "Camera.h"
#include "Animation2D.h"
#include "Renderer2D.h"
#include "Collider2D.h"
#include "BoxCollider2D.h"
#include "CircleCollider2D.h"
#include "Rigidbody2D.h"
#include "Entity.h"
#include "Event.h"
#include "Group.h"
#include "Pool.h"
#include "Debug.h"
#include "Asset.h"
#include "Input.h"
#include "Engine.h"
#include "Scene.h"




static MemberInfo LuaMember(const std::string& type, size_t offset)
{
	MemberInfo info;
	info.memberType = type;
	info.offset = offset;
	return info;
}

static PClass* CreateLuaMetaClass(const std::string& name, long flag = EngineMetaFlag::Class)
{
	PType* type = GetType(name);
	if (type != nullptr)
	{
		SetTypeFlag(type, flag);
		return reinterpret_cast<PClass*>(type);
	}
	PClass* newClass = CreateNewClass(name);
	SetTypeFlag(reinterpret_cast<PType*>(newClass), flag);
	return newClass;
}

static PStatic* CreateLuaMetaStatic(const std::string& name, long flag = EngineMetaFlag::StaticGlobal)
{
	PType* type = GetType(name);
	if (type != nullptr)
	{
		SetTypeFlag(type, flag);
		return reinterpret_cast<PStatic*>(type);
	}
	PStatic* newStatic = CreateNewStatic(name);
	SetTypeFlag(reinterpret_cast<PType*>(newStatic), flag);
	return newStatic;
}

static void RegisterComponentData()
{
	std::string name = "Transform";
	PClass* GlobalTransformData = CreateLuaMetaClass(name + "Data", EngineMetaFlag::ComponentData);
	AddMember(GlobalTransformData, "position", LuaMember("PVector3", offsetof(TransformData, position)), EngineMetaFlag::ComponentData);
	AddMember(GlobalTransformData, "bitmask", LuaMember("unsigned int", offsetof(TransformData, bitmask)), EngineMetaFlag::ComponentData);
	AddMember(GlobalTransformData, "rotation", LuaMember("PVector3", offsetof(TransformData, rotation)), EngineMetaFlag::ComponentData);
	AddMember(GlobalTransformData, "unused1", LuaMember("unsigned int", offsetof(TransformData, unused1)), EngineMetaFlag::ComponentData);
	AddMember(GlobalTransformData, "scale", LuaMember("PVector3", offsetof(TransformData, scale)), EngineMetaFlag::ComponentData);
	AddMember(GlobalTransformData, "unused2", LuaMember("unsigned int", offsetof(TransformData, unused2)), EngineMetaFlag::ComponentData);
	PStatic* GlobalTransform = CreateLuaMetaStatic("Transform", EngineMetaFlag::Component);
	auto info_Add = GeGlobalMethodInfo(&Transform_Add);
	info_Add.memberName.push_back("ID");
	auto info_Get = GeGlobalMethodInfo(&Transform_Get);
	info_Get.memberName.push_back("ID");
	auto info_Has = GeGlobalMethodInfo(&Transform_Has);
	info_Has.memberName.push_back("ID");
	AddGlobalMethod(GlobalTransform, name + "_Add", info_Add, EngineMetaFlag::Component);
	AddGlobalMethod(GlobalTransform, name + "_Get", info_Get, EngineMetaFlag::Component);
	AddGlobalMethod(GlobalTransform, name + "_Has", info_Has, EngineMetaFlag::Component);




	PEnum* GlobalMotionType = CreateNewEnum("MotionType");
	AddEnum(GlobalMotionType, "Static");
	AddEnum(GlobalMotionType, "Kinematic");
	AddEnum(GlobalMotionType, "Dynamic");

	name = "Renderer2D";
	PClass* renderer2DData = CreateLuaMetaClass(name + "Data", EngineMetaFlag::ComponentData);
	AddMember(renderer2DData, "TilingX", LuaMember("float", offsetof(Renderer2DData, TilingX)), EngineMetaFlag::ComponentData);
	AddMember(renderer2DData, "TilingY", LuaMember("float", offsetof(Renderer2DData, TilingY)), EngineMetaFlag::ComponentData);
	AddMember(renderer2DData, "OffsetX", LuaMember("float", offsetof(Renderer2DData, OffsetX)), EngineMetaFlag::ComponentData);
	AddMember(renderer2DData, "OffsetY", LuaMember("float", offsetof(Renderer2DData, OffsetY)), EngineMetaFlag::ComponentData);
	PStatic* GlobalRenderer2D = CreateLuaMetaStatic(name, EngineMetaFlag::Component);
	info_Add = GeGlobalMethodInfo(&Renderer2D_Add);
	info_Add.memberName.push_back("ID");
	info_Get = GeGlobalMethodInfo(&Renderer2D_Get);
	info_Get.memberName.push_back("ID");
	info_Has = GeGlobalMethodInfo(&Renderer2D_Has);
	info_Has.memberName.push_back("ID");
	auto info_SetTextrue = GeGlobalMethodInfo(&Renderer2D_SetTexture);
	info_SetTextrue.memberName.push_back("ID");
	info_SetTextrue.memberName.push_back("textureName");
	AddGlobalMethod(GlobalRenderer2D, name + "_Add", info_Add, EngineMetaFlag::Component);
	AddGlobalMethod(GlobalRenderer2D, name + "_Get", info_Get, EngineMetaFlag::Component);
	AddGlobalMethod(GlobalRenderer2D, name + "_Has", info_Has, EngineMetaFlag::Component);
	AddGlobalMethod(GlobalRenderer2D, "Renderer2D_SetTexture", info_SetTextrue, EngineMetaFlag::Component);


	name = "Camera";
	PClass* cameraData = CreateLuaMetaClass(name + "Data", EngineMetaFlag::ComponentData);
	AddMember(cameraData, "FovY", LuaMember("float", offsetof(CameraData, FovY)), EngineMetaFlag::ComponentData);
	AddMember(cameraData, "NearZ", LuaMember("float", offsetof(CameraData, NearZ)), EngineMetaFlag::ComponentData);
	AddMember(cameraData, "FarZ", LuaMember("float", offsetof(CameraData, FarZ)), EngineMetaFlag::ComponentData);
	AddMember(cameraData, "ZoomLevel", LuaMember("float", offsetof(CameraData, ZoomLevel)), EngineMetaFlag::ComponentData);
	PStatic* GlobalCamera = CreateLuaMetaStatic(name, EngineMetaFlag::Component);
	info_Add = GeGlobalMethodInfo(&Camera_Add);
	info_Add.memberName.push_back("ID");
	info_Get = GeGlobalMethodInfo(&Camera_Get);
	info_Get.memberName.push_back("ID");
	info_Has = GeGlobalMethodInfo(&Camera_Has);
	info_Has.memberName.push_back("ID");
	AddGlobalMethod(GlobalCamera, name + "_Add", info_Add, EngineMetaFlag::Component);
	AddGlobalMethod(GlobalCamera, name + "_Get", info_Get, EngineMetaFlag::Component);
	AddGlobalMethod(GlobalCamera, name + "_Has", info_Has, EngineMetaFlag::Component);


	//CreateLuaMetaClass("CameraData", EngineMetaFlag::ComponentData);
	//
	//PClass* boxCollider2DData = CreateLuaMetaClass("BoxCollider2DData", EngineMetaFlag::ComponentData);
	//AddMember(boxCollider2DData, "Center", LuaMember("PVector2", offsetof(ECS::BoxCollider2D::BoxCollider2DData, Center)), EngineMetaFlag::LuaBind);
	//AddMember(boxCollider2DData, "Offset", LuaMember("PVector2", offsetof(ECS::BoxCollider2D::BoxCollider2DData, Offset)), EngineMetaFlag::LuaBind);
	//
	//PClass* circleCollider2DData = CreateLuaMetaClass("CircleCollider2DData", EngineMetaFlag::ComponentData);
	//AddMember(circleCollider2DData, "Center", LuaMember("PVector2", offsetof(ECS::CircleCollider2D::CircleCollider2DData, Center)), EngineMetaFlag::LuaBind);
	//AddMember(circleCollider2DData, "Radius", LuaMember("float", offsetof(ECS::CircleCollider2D::CircleCollider2DData, Radius)), EngineMetaFlag::LuaBind);
	//
	//PClass* rigidbody2DData = CreateLuaMetaClass("Rigidbody2DData", EngineMetaFlag::ComponentData);
	//AddMember(rigidbody2DData, "type", LuaMember("MotionType", offsetof(ECS::Rigidbody2D::Rigidbody2DData, type)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "Active", LuaMember("bool", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Active)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "Kinematic", LuaMember("bool", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Kinematic)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "AutoSleep", LuaMember("bool", offsetof(ECS::Rigidbody2D::Rigidbody2DData, AutoSleep)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "Sensor", LuaMember("bool", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Sensor)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "Gravity", LuaMember("float", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Gravity)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "Restitution", LuaMember("float", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Restitution)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "Friction", LuaMember("float", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Friction)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "LinearDamping", LuaMember("float", offsetof(ECS::Rigidbody2D::Rigidbody2DData, LinearDamping)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "velocity", LuaMember("PVector3", offsetof(ECS::Rigidbody2D::Rigidbody2DData, velocity)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "impulse", LuaMember("PVector3", offsetof(ECS::Rigidbody2D::Rigidbody2DData, impulse)), EngineMetaFlag::LuaBind);
	//AddMember(rigidbody2DData, "force", LuaMember("PVector3", offsetof(ECS::Rigidbody2D::Rigidbody2DData, force)), EngineMetaFlag::LuaBind);
}
BindManager::BindManager()
{

}

BindManager::~BindManager()
{

}

void BindManager::Initialize()
{
	PClass* GlobalVector2 = CreateLuaMetaClass("PVector2", EngineMetaFlag::ClassData);
	AddMember(GlobalVector2, "x", LuaMember("float", offsetof(PVector2, x)), EngineMetaFlag::ClassData);
	AddMember(GlobalVector2, "y", LuaMember("float", offsetof(PVector2, y)), EngineMetaFlag::ClassData);

	PClass* GlobalVector3Data = CreateLuaMetaClass("PVector3", EngineMetaFlag::ClassData);
	AddMember(GlobalVector3Data, "x", LuaMember("float", offsetof(PVector3, x)), EngineMetaFlag::ClassData);
	AddMember(GlobalVector3Data, "y", LuaMember("float", offsetof(PVector3, y)), EngineMetaFlag::ClassData);
	AddMember(GlobalVector3Data, "z", LuaMember("float", offsetof(PVector3, z)), EngineMetaFlag::ClassData);
	PStatic* GlobalVector3 = CreateLuaMetaStatic("PVector3Function", EngineMetaFlag::Class);
	AddGlobalMethod(GlobalVector3, "PVector3_Normalize", GeGlobalMethodInfo(&PVector3_Normalize), EngineMetaFlag::Class);

	//Engine
	MethodInfo info;
	PStatic* globalCreate = CreateLuaMetaStatic("Engine", EngineMetaFlag::Class);
	info = GeGlobalMethodInfo(&Engine_BackgroundColor);
	info.memberName.push_back("colorR");
	info.memberName.push_back("colorG");
	info.memberName.push_back("colorB");
	AddGlobalMethod(globalCreate, "Engine_BackgroundColor", info, EngineMetaFlag::Class);

	//SceneObject
	PStatic* globalScene = CreateLuaMetaStatic("Scene", EngineMetaFlag::Class);
	info = GeGlobalMethodInfo(&Scene_Change);
	info.memberName.push_back("sceneName");
	AddGlobalMethod(globalScene, "Scene_Change", info, EngineMetaFlag::Class);

	//Asset
	PStatic* globalAsset = CreateLuaMetaStatic("Asset", EngineMetaFlag::Class);
	info = GeGlobalMethodInfo(&Asset_Import);
	info.memberName.push_back("assetPath");
	AddGlobalMethod(globalAsset, "Asset_Import", info, EngineMetaFlag::Class);

	//Input
	PStatic* globalInput = CreateLuaMetaStatic("Input", EngineMetaFlag::Class);
	auto Keyinfo = GeGlobalMethodInfo(&Input_Key);
	auto KeyDowninfo = GeGlobalMethodInfo(&Input_KeyDown);
	auto KeyUpinfo = GeGlobalMethodInfo(&Input_KeyUp);
	Keyinfo.memberName.push_back("keyNumber");
	KeyDowninfo.memberName.push_back("keyNumber");
	KeyUpinfo.memberName.push_back("keyNumber");
	AddGlobalMethod(globalInput, "Input_Key", Keyinfo, EngineMetaFlag::Class);
	AddGlobalMethod(globalInput, "Input_KeyDown", KeyDowninfo, EngineMetaFlag::Class);
	AddGlobalMethod(globalInput, "Input_KeyUp", KeyUpinfo, EngineMetaFlag::Class);
	AddGlobalMethod(globalInput, "Input_GetMousePositionX", GeGlobalMethodInfo(&Input_GetMousePositionX), EngineMetaFlag::Class);
	AddGlobalMethod(globalInput, "Input_GetMousePositionY", GeGlobalMethodInfo(&Input_GetMousePositionY), EngineMetaFlag::Class);

	//Debug
	PStatic* globaDebug = CreateLuaMetaStatic("Debug", EngineMetaFlag::Class);
	auto Debuginfo = GeGlobalMethodInfo(&Debug_Log);
	auto DebugErrorinfo = GeGlobalMethodInfo(&Debug_LogError);
	auto DebugWarninginfo = GeGlobalMethodInfo(&Debug_LogWarning);
	auto DebugLineinfo = GeGlobalMethodInfo(&Debug_Line);
	Debuginfo.memberName.push_back("logString");
	DebugErrorinfo.memberName.push_back("logString");
	DebugWarninginfo.memberName.push_back("logString");
	DebugLineinfo.memberName.push_back("startPoint");
	DebugLineinfo.memberName.push_back("endPoint");
	DebugLineinfo.memberName.push_back("color");
	AddGlobalMethod(globaDebug, "Debug_Log", Debuginfo, EngineMetaFlag::Class);
	AddGlobalMethod(globaDebug, "Debug_LogError", DebugErrorinfo, EngineMetaFlag::Class);
	AddGlobalMethod(globaDebug, "Debug_LogWarning", DebugWarninginfo, EngineMetaFlag::Class);
	AddGlobalMethod(globaDebug, "Debug_Line", DebugLineinfo, EngineMetaFlag::Class);


	//Entity;
	PStatic* globalEntity = CreateLuaMetaStatic("Entity", EngineMetaFlag::Class);
	auto EntityCreate = GeGlobalMethodInfo(&Entity_Create);
	EntityCreate.memberName.push_back("ScriptName");
	auto EntityDestroy = GeGlobalMethodInfo(&Entity_Destroy);
	EntityDestroy.memberName.push_back("ID");
	auto EntityGetActive = GeGlobalMethodInfo(&Entity_GetActive);
	EntityGetActive.memberName.push_back("ID");
	auto EntitySetActive = GeGlobalMethodInfo(&Entity_SetActive);
	EntitySetActive.memberName.push_back("ID");
	EntitySetActive.memberName.push_back("IsActive");
	AddGlobalMethod(globalEntity, "Entity_Create", EntityCreate, EngineMetaFlag::Class);
	AddGlobalMethod(globalEntity, "Entity_Destroy", EntityDestroy, EngineMetaFlag::Class);
	AddGlobalMethod(globalEntity, "Entity_GetActive", EntityGetActive, EngineMetaFlag::Class);
	AddGlobalMethod(globalEntity, "Entity_SetActive", EntitySetActive, EngineMetaFlag::Class);








	//PStatic* globalTransform = CreateLuaMetaStatic("Transform", EngineMetaFlag::Component);
	//AddGlobalMethod(globalTransform, "Transform_Add", GeGlobalMethodInfo(&Transform_Add), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalTransform, "Transform_Get", GeGlobalMethodInfo(&Transform_Get), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalTransform, "Transform_Has", GeGlobalMethodInfo(&Transform_Has), EngineMetaFlag::LuaBind);
	//
	//
	//PStatic* globalRenderer2D = CreateLuaMetaStatic("Renderer2D", EngineMetaFlag::Component);
	//AddGlobalMethod(globalRenderer2D, "AddComponent", GeGlobalMethodInfo(&ECS::Renderer2D::AddComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalRenderer2D, "GetComponent", GeGlobalMethodInfo(&ECS::Renderer2D::GetComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalRenderer2D, "HasComponent", GeGlobalMethodInfo(&ECS::Renderer2D::HasComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalRenderer2D, "SetTexture", GeGlobalMethodInfo(&ECS::Renderer2D::SetTexture), EngineMetaFlag::LuaBind);
	//
	//
	//PStatic* globalCamera = CreateLuaMetaStatic("Camera", EngineMetaFlag::Component);
	//AddGlobalMethod(globalCamera, "AddComponent", GeGlobalMethodInfo(&ECS::Camera::AddComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalCamera, "GetComponent", GeGlobalMethodInfo(&ECS::Camera::GetComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalCamera, "HasComponent", GeGlobalMethodInfo(&ECS::Camera::HasComponent), EngineMetaFlag::LuaBind);
	//
	//PStatic* globalAnimation2D = CreateLuaMetaStatic("Animation2D");
	//AddGlobalMethod(globalAnimation2D, "Add", GeGlobalMethodInfo(&ECS::Animation2D::Add), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalAnimation2D, "Create", GeGlobalMethodInfo(&ECS::Animation2D::Create), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalAnimation2D, "Play", GeGlobalMethodInfo(&ECS::Animation2D::Play), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalAnimation2D, "Stop", GeGlobalMethodInfo(&ECS::Animation2D::Stop), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalAnimation2D, "Reset", GeGlobalMethodInfo(&ECS::Animation2D::Reset), EngineMetaFlag::LuaBind);
	//
	//PStatic* globalBoxCollider2D = CreateLuaMetaStatic("BoxCollider2D", EngineMetaFlag::Component);
	//AddGlobalMethod(globalBoxCollider2D, "AddComponent", GeGlobalMethodInfo(&ECS::BoxCollider2D::AddComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalBoxCollider2D, "GetComponent", GeGlobalMethodInfo(&ECS::BoxCollider2D::GetComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalBoxCollider2D, "HasComponent", GeGlobalMethodInfo(&ECS::BoxCollider2D::HasComponent), EngineMetaFlag::LuaBind);
	//
	//PStatic* globalCircleCollider2D = CreateLuaMetaStatic("CircleCollider2D", EngineMetaFlag::Component);
	//AddGlobalMethod(globalCircleCollider2D, "AddComponent", GeGlobalMethodInfo(&ECS::CircleCollider2D::AddComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalCircleCollider2D, "GetComponent", GeGlobalMethodInfo(&ECS::CircleCollider2D::GetComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalCircleCollider2D, "HasComponent", GeGlobalMethodInfo(&ECS::CircleCollider2D::HasComponent), EngineMetaFlag::LuaBind);
	//
	//PStatic* globalRigidbody2D = CreateLuaMetaStatic("Rigidbody2D", EngineMetaFlag::Component);
	//AddGlobalMethod(globalRigidbody2D, "AddComponent", GeGlobalMethodInfo(&ECS::Rigidbody2D::AddComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalRigidbody2D, "GetComponent", GeGlobalMethodInfo(&ECS::Rigidbody2D::GetComponent), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalRigidbody2D, "HasComponent", GeGlobalMethodInfo(&ECS::Rigidbody2D::HasComponent), EngineMetaFlag::LuaBind);
	//
	//AddGlobalMethod(globalRigidbody2D, "SetLayer", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetLayer), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalRigidbody2D, "SetPosition", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetPosition), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalRigidbody2D, "SetRotation", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetRotation), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalRigidbody2D, "LockPosition", GeGlobalMethodInfo(&ECS::Rigidbody2D::LockPosition), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalRigidbody2D, "LockRotation", GeGlobalMethodInfo(&ECS::Rigidbody2D::LockRotation), EngineMetaFlag::LuaBind);
	//
	////PStatic* globalLuaEvent = CreateLuaMetaStatic("Event");
	////AddGlobalMethod(globalLuaEvent, "BindEvent", GeGlobalMethodInfo(&ECS::Event::BindEvent), EngineMetaFlag::LuaBind);
	////AddGlobalMethod(globalLuaEvent, "CallEvent", GeGlobalMethodInfo(&ECS::Event::CallEvent), EngineMetaFlag::LuaBind);
	//
	//PStatic* globalGroup = CreateLuaMetaStatic("Group");
	//AddGlobalMethod(globalGroup, "CreateEntity", GeGlobalMethodInfo(&ECS::Group::CreateEntity), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalGroup, "Set", GeGlobalMethodInfo(&ECS::Group::Set), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalGroup, "Remove", GeGlobalMethodInfo(&ECS::Group::Remove), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalGroup, "Clear", GeGlobalMethodInfo(&ECS::Group::Clear), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalGroup, "Get", GeGlobalMethodInfo(&ECS::Group::Get), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalGroup, "Count", GeGlobalMethodInfo(&ECS::Group::Count), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalGroup, "First", GeGlobalMethodInfo(&ECS::Group::First), EngineMetaFlag::LuaBind);
	//
	//PStatic* globalPool = CreateLuaMetaStatic("Pool");
	//AddGlobalMethod(globalPool, "Pool_Active", GeGlobalMethodInfo(Pool_Active), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalPool, "Pool_Disable", GeGlobalMethodInfo(&Pool_Disable), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalPool, "Pool_Clear", GeGlobalMethodInfo(&Pool_Clear), EngineMetaFlag::LuaBind);
	//
	//
	//PStatic* globalEntity = CreateLuaMetaStatic("Entity");
	//AddGlobalMethod(globalEntity, "Entity_Create", GeGlobalMethodInfo(&Entity_Create), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalEntity, "Entity_Destroy", GeGlobalMethodInfo(&Entity_Destroy), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalEntity, "Entity_GetActive", GeGlobalMethodInfo(&Entity_GetActive), EngineMetaFlag::LuaBind);
	//AddGlobalMethod(globalEntity, "Entity_SetActive", GeGlobalMethodInfo(&Entity_SetActive), EngineMetaFlag::LuaBind);
	////AddGlobalMethod(globalEntity, "GetValue", GeGlobalMethodInfo(&Entity_Destroy), EngineMetaFlag::LuaBind);
	////AddGlobalMethod(globalEntity, "SetValue", GeGlobalMethodInfo(&Entity_Destroy), EngineMetaFlag::LuaBind);
	//
	//
	RegisterComponentData();
	//BindEnum();
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
