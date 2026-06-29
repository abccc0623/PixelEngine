#include "pch.h"
#include "BindManager.h"
#include "Type/GlobalEnum.h"

#include "PixelMetaAPI.h"
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


static MemberInfo LuaMember(const std::string& type, size_t offset)
{
	MemberInfo info;
	info.memberType = type;
	info.offset = offset;
	return info;
}

static PClass* CreateLuaMetaClass(const std::string& name)
{
	PType* type = GetType(name);
	if (type != nullptr)
	{
		return reinterpret_cast<PClass*>(type);
	}
	return CreateNewClass(name);
}

static void RegisterComponentData()
{
	CreateLuaMetaClass("Vector2");
	PClass* GlobalVector3 = CreateLuaMetaClass("Vector3");

	CreateLuaMetaClass("MotionType");

	PClass* transformData = CreateLuaMetaClass("TransformData");
	AddMember(transformData, "position", LuaMember("Vector3", offsetof(ECS::Transform::TransformData, position)), MetaFlag::LUABIND);
	AddMember(transformData, "rotation", LuaMember("Vector3", offsetof(ECS::Transform::TransformData, rotation)), MetaFlag::LUABIND);
	AddMember(transformData, "scale", LuaMember("Vector3", offsetof(ECS::Transform::TransformData, scale)), MetaFlag::LUABIND);

	PClass* renderer2DData = CreateLuaMetaClass("Renderer2DData");
	AddMember(renderer2DData, "TilingX", LuaMember("float", offsetof(ECS::Renderer2D::Renderer2DData, TilingX)), MetaFlag::LUABIND);
	AddMember(renderer2DData, "TilingY", LuaMember("float", offsetof(ECS::Renderer2D::Renderer2DData, TilingY)), MetaFlag::LUABIND);
	AddMember(renderer2DData, "OffsetX", LuaMember("float", offsetof(ECS::Renderer2D::Renderer2DData, OffsetX)), MetaFlag::LUABIND);
	AddMember(renderer2DData, "OffsetY", LuaMember("float", offsetof(ECS::Renderer2D::Renderer2DData, OffsetY)), MetaFlag::LUABIND);

	CreateLuaMetaClass("CameraData");

	PClass* boxCollider2DData = CreateLuaMetaClass("BoxCollider2DData");
	AddMember(boxCollider2DData, "Center", LuaMember("Vector2", offsetof(ECS::BoxCollider2D::BoxCollider2DData, Center)), MetaFlag::LUABIND);
	AddMember(boxCollider2DData, "Offset", LuaMember("Vector2", offsetof(ECS::BoxCollider2D::BoxCollider2DData, Offset)), MetaFlag::LUABIND);

	PClass* circleCollider2DData = CreateLuaMetaClass("CircleCollider2DData");
	AddMember(circleCollider2DData, "Center", LuaMember("Vector2", offsetof(ECS::CircleCollider2D::CircleCollider2DData, Center)), MetaFlag::LUABIND);
	AddMember(circleCollider2DData, "Radius", LuaMember("float", offsetof(ECS::CircleCollider2D::CircleCollider2DData, Radius)), MetaFlag::LUABIND);

	PClass* rigidbody2DData = CreateLuaMetaClass("Rigidbody2DData");
	AddMember(rigidbody2DData, "type", LuaMember("MotionType", offsetof(ECS::Rigidbody2D::Rigidbody2DData, type)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "Active", LuaMember("bool", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Active)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "Kinematic", LuaMember("bool", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Kinematic)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "AutoSleep", LuaMember("bool", offsetof(ECS::Rigidbody2D::Rigidbody2DData, AutoSleep)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "Sensor", LuaMember("bool", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Sensor)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "Gravity", LuaMember("float", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Gravity)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "Restitution", LuaMember("float", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Restitution)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "Friction", LuaMember("float", offsetof(ECS::Rigidbody2D::Rigidbody2DData, Friction)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "LinearDamping", LuaMember("float", offsetof(ECS::Rigidbody2D::Rigidbody2DData, LinearDamping)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "velocity", LuaMember("Vector3", offsetof(ECS::Rigidbody2D::Rigidbody2DData, velocity)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "impulse", LuaMember("Vector3", offsetof(ECS::Rigidbody2D::Rigidbody2DData, impulse)), MetaFlag::LUABIND);
	AddMember(rigidbody2DData, "force", LuaMember("Vector3", offsetof(ECS::Rigidbody2D::Rigidbody2DData, force)), MetaFlag::LUABIND);
}
BindManager::BindManager()
{

}

BindManager::~BindManager()
{

}

void BindManager::Initialize()
{
	PStatic* globalCreate = CreateNewStatic("Engine");
	AddGlobalMethod(globalCreate, "BindLuaEvent", GeGlobalMethodInfo(&BindLuaEvent), MetaFlag::LUABIND);
	//AddGlobalMethod(globalCreate, "CallLuaEvent", GeGlobalMethodInfo(&CallLuaEvent), MetaFlag::LUABIND);

	AddGlobalMethod(globalCreate, "BackgroundColor", GeGlobalMethodInfo(&BackgroundColor), MetaFlag::LUABIND);

	PStatic* globalScene = CreateNewStatic("Scene");
	AddGlobalMethod(globalScene, "ChangeScene", GeGlobalMethodInfo(&ChangeScene), MetaFlag::LUABIND);

	PStatic* globalAsset = CreateNewStatic("Asset");
	AddGlobalMethod(globalAsset, "Asset_Import", GeGlobalMethodInfo(&Asset_Import), MetaFlag::LUABIND);

	PStatic* globalInput = CreateNewStatic("Input");
	AddGlobalMethod(globalInput, "GetKey", GeGlobalMethodInfo(&GetKey), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput, "GetKeyDown", GeGlobalMethodInfo(&GetKeyDown), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput, "GetKeyUp", GeGlobalMethodInfo(&GetKeyUp), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput, "GetMousePosition_X", GeGlobalMethodInfo(&GetMousePosition_X), MetaFlag::LUABIND);
	AddGlobalMethod(globalInput, "GetMousePosition_Y", GeGlobalMethodInfo(&GetMousePosition_Y), MetaFlag::LUABIND);

	PStatic* globaDebug = CreateNewStatic("Debug");
	AddGlobalMethod(globaDebug, "Debug_Log", GeGlobalMethodInfo(&Debug_Log), MetaFlag::LUABIND);
	AddGlobalMethod(globaDebug, "Debug_LogError", GeGlobalMethodInfo(&Debug_LogError), MetaFlag::LUABIND);
	AddGlobalMethod(globaDebug, "Debug_LogWarning", GeGlobalMethodInfo(&Debug_LogWarning), MetaFlag::LUABIND);
	AddGlobalMethod(globaDebug, "Debug_Line", GeGlobalMethodInfo(&Debug_Line), MetaFlag::LUABIND);


	PStatic* globalTransform = CreateNewStatic("Transform");
	AddGlobalMethod(globalTransform, "AddComponent", GeGlobalMethodInfo(&ECS::Transform::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalTransform, "GetComponent", GeGlobalMethodInfo(&ECS::Transform::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalTransform, "HasComponent", GeGlobalMethodInfo(&ECS::Transform::HasComponent), MetaFlag::LUABIND);


	PStatic* globalRenderer2D = CreateNewStatic("Renderer2D");
	AddGlobalMethod(globalRenderer2D, "AddComponent", GeGlobalMethodInfo(&ECS::Renderer2D::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "GetComponent", GeGlobalMethodInfo(&ECS::Renderer2D::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "HasComponent", GeGlobalMethodInfo(&ECS::Renderer2D::HasComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRenderer2D, "SetTexture", GeGlobalMethodInfo(&ECS::Renderer2D::SetTexture), MetaFlag::LUABIND);


	PStatic* globalCamera = CreateNewStatic("Camera");
	AddGlobalMethod(globalCamera, "AddComponent", GeGlobalMethodInfo(&ECS::Camera::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalCamera, "GetComponent", GeGlobalMethodInfo(&ECS::Camera::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalCamera, "HasComponent", GeGlobalMethodInfo(&ECS::Camera::HasComponent), MetaFlag::LUABIND);

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

	PStatic* globalCircleCollider2D = CreateNewStatic("CircleCollider2D");
	AddGlobalMethod(globalCircleCollider2D, "AddComponent", GeGlobalMethodInfo(&ECS::CircleCollider2D::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalCircleCollider2D, "GetComponent", GeGlobalMethodInfo(&ECS::CircleCollider2D::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalCircleCollider2D, "HasComponent", GeGlobalMethodInfo(&ECS::CircleCollider2D::HasComponent), MetaFlag::LUABIND);

	PStatic* globalRigidbody2D = CreateNewStatic("Rigidbody2D");
	AddGlobalMethod(globalRigidbody2D, "AddComponent", GeGlobalMethodInfo(&ECS::Rigidbody2D::AddComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRigidbody2D, "GetComponent", GeGlobalMethodInfo(&ECS::Rigidbody2D::GetComponent), MetaFlag::LUABIND);
	AddGlobalMethod(globalRigidbody2D, "HasComponent", GeGlobalMethodInfo(&ECS::Rigidbody2D::HasComponent), MetaFlag::LUABIND);

	AddGlobalMethod(globalRigidbody2D, "SetLayer", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetLayer), MetaFlag::LUABIND);
	AddGlobalMethod(globalRigidbody2D, "SetPosition", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetPosition), MetaFlag::LUABIND);
	AddGlobalMethod(globalRigidbody2D, "SetRotation", GeGlobalMethodInfo(&ECS::Rigidbody2D::SetRotation), MetaFlag::LUABIND);
	AddGlobalMethod(globalRigidbody2D, "LockPosition", GeGlobalMethodInfo(&ECS::Rigidbody2D::LockPosition), MetaFlag::LUABIND);
	AddGlobalMethod(globalRigidbody2D, "LockRotation", GeGlobalMethodInfo(&ECS::Rigidbody2D::LockRotation), MetaFlag::LUABIND);

	PStatic* globalLuaEvent = CreateNewStatic("Event");
	AddGlobalMethod(globalLuaEvent, "BindEvent", GeGlobalMethodInfo(&ECS::Event::BindEvent), MetaFlag::LUABIND);
	AddGlobalMethod(globalLuaEvent, "CallEvent", GeGlobalMethodInfo(&ECS::Event::CallEvent), MetaFlag::LUABIND);

	PStatic* globalGroup = CreateNewStatic("Group");
	AddGlobalMethod(globalGroup, "CreateEntity", GeGlobalMethodInfo(&ECS::Group::CreateEntity), MetaFlag::LUABIND);
	AddGlobalMethod(globalGroup, "Set", GeGlobalMethodInfo(&ECS::Group::Set), MetaFlag::LUABIND);
	AddGlobalMethod(globalGroup, "Remove", GeGlobalMethodInfo(&ECS::Group::Remove), MetaFlag::LUABIND);
	AddGlobalMethod(globalGroup, "Clear", GeGlobalMethodInfo(&ECS::Group::Clear), MetaFlag::LUABIND);
	AddGlobalMethod(globalGroup, "Get", GeGlobalMethodInfo(&ECS::Group::Get), MetaFlag::LUABIND);
	AddGlobalMethod(globalGroup, "Count", GeGlobalMethodInfo(&ECS::Group::Count), MetaFlag::LUABIND);
	AddGlobalMethod(globalGroup, "First", GeGlobalMethodInfo(&ECS::Group::First), MetaFlag::LUABIND);

	PStatic* globalPool = CreateNewStatic("Pool");
	AddGlobalMethod(globalPool, "Pool_Active", GeGlobalMethodInfo(Pool_Active), MetaFlag::LUABIND);
	AddGlobalMethod(globalPool, "Pool_Disable", GeGlobalMethodInfo(&Pool_Disable), MetaFlag::LUABIND);
	AddGlobalMethod(globalPool, "Pool_Clear", GeGlobalMethodInfo(&Pool_Clear), MetaFlag::LUABIND);


	PStatic* globalEntity = CreateNewStatic("Entity");
	AddGlobalMethod(globalEntity, "Entity_Create", GeGlobalMethodInfo(&Entity_Create), MetaFlag::LUABIND);
	AddGlobalMethod(globalEntity, "Entity_Destroy", GeGlobalMethodInfo(&Entity_Destroy), MetaFlag::LUABIND);
	AddGlobalMethod(globalEntity, "Entity_GetActive", GeGlobalMethodInfo(&Entity_GetActive), MetaFlag::LUABIND);
	AddGlobalMethod(globalEntity, "Entity_SetActive", GeGlobalMethodInfo(&Entity_SetActive), MetaFlag::LUABIND);
	//AddGlobalMethod(globalEntity, "GetValue", GeGlobalMethodInfo(&Entity_Destroy), MetaFlag::LUABIND);
	//AddGlobalMethod(globalEntity, "SetValue", GeGlobalMethodInfo(&Entity_Destroy), MetaFlag::LUABIND);


	RegisterComponentData();
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
