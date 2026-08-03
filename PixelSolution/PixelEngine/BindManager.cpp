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
#include "UIImage.h"
#include "UIText.h"
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
	PClass* Data = nullptr;
	PStatic* Static = nullptr;
	std::string name = "Transform";

	PClass* GlobalTransformData = CreateLuaMetaClass(name + "Data", EngineMetaFlag::ComponentData);
	AddMember(GlobalTransformData, "position", LuaMember("PVector3", offsetof(TransformData, position)), EngineMetaFlag::Public);
	AddMember(GlobalTransformData, "thisID", LuaMember("unsigned int", offsetof(TransformData, thisID)), EngineMetaFlag::Private);
	AddMember(GlobalTransformData, "rotation", LuaMember("PVector3", offsetof(TransformData, rotation)), EngineMetaFlag::Public);
	AddMember(GlobalTransformData, "bitmask", LuaMember("unsigned int", offsetof(TransformData, bitmask)), EngineMetaFlag::Private);
	AddMember(GlobalTransformData, "scale", LuaMember("PVector3", offsetof(TransformData, scale)), EngineMetaFlag::Public);
	AddMember(GlobalTransformData, "unused2", LuaMember("unsigned int", offsetof(TransformData, unused2)), EngineMetaFlag::Private);
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
	AddMember(renderer2DData, "thisID", LuaMember("unsigned int", offsetof(Renderer2DData, thisID)), EngineMetaFlag::Private);
	AddMember(renderer2DData, "TilingX", LuaMember("float", offsetof(Renderer2DData, TilingX)), EngineMetaFlag::Public);
	AddMember(renderer2DData, "TilingY", LuaMember("float", offsetof(Renderer2DData, TilingY)), EngineMetaFlag::Public);
	AddMember(renderer2DData, "OffsetX", LuaMember("float", offsetof(Renderer2DData, OffsetX)), EngineMetaFlag::Public);
	AddMember(renderer2DData, "OffsetY", LuaMember("float", offsetof(Renderer2DData, OffsetY)), EngineMetaFlag::Public);
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

	name = "UIImage";
	Data = CreateLuaMetaClass(name + "Data", EngineMetaFlag::ComponentData);
	AddMember(Data, "thisID", LuaMember("unsigned int", offsetof(UIImageData, thisID)), EngineMetaFlag::Private);
	AddMember(Data, "order", LuaMember("int", offsetof(UIImageData, order)), EngineMetaFlag::Public);
	Static = CreateLuaMetaStatic(name, EngineMetaFlag::Component);
	info_Add = GeGlobalMethodInfo(&UIImage_Add);
	info_Add.memberName.push_back("ID");
	info_Get = GeGlobalMethodInfo(&UIImage_Get);
	info_Get.memberName.push_back("ID");
	info_Has = GeGlobalMethodInfo(&UIImage_Has);
	info_Has.memberName.push_back("ID");
	auto info_UIImageSetTexture = GeGlobalMethodInfo(&UIImage_SetTexture);
	info_UIImageSetTexture.memberName.push_back("ID");
	info_UIImageSetTexture.memberName.push_back("textureName");
	AddGlobalMethod(Static, name + "_Add", info_Add, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Get", info_Get, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Has", info_Has, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_SetTexture", info_UIImageSetTexture, EngineMetaFlag::Component);

	name = "UIText";
	Data = CreateLuaMetaClass(name + "Data", EngineMetaFlag::ComponentData);
	AddMember(Data, "thisID", LuaMember("unsigned int", offsetof(UITextData, thisID)), EngineMetaFlag::Private);
	Static = CreateLuaMetaStatic(name, EngineMetaFlag::Component);
	info_Add = GeGlobalMethodInfo(&UIText_Add);
	info_Add.memberName.push_back("ID");
	info_Get = GeGlobalMethodInfo(&UIText_Get);
	info_Get.memberName.push_back("ID");
	info_Has = GeGlobalMethodInfo(&UIText_Has);
	info_Has.memberName.push_back("ID");
	AddGlobalMethod(Static, name + "_Add", info_Add, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Get", info_Get, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Has", info_Has, EngineMetaFlag::Component);

	name = "Camera";
	Data = CreateLuaMetaClass(name + "Data", EngineMetaFlag::ComponentData);
	AddMember(Data, "thisID", LuaMember("unsigned int", offsetof(CameraData, thisID)), EngineMetaFlag::Private);
	AddMember(Data, "FovY", LuaMember("float", offsetof(CameraData, FovY)), EngineMetaFlag::Public);
	AddMember(Data, "NearZ", LuaMember("float", offsetof(CameraData, NearZ)), EngineMetaFlag::Public);
	AddMember(Data, "FarZ", LuaMember("float", offsetof(CameraData, FarZ)), EngineMetaFlag::Public);
	AddMember(Data, "ZoomLevel", LuaMember("float", offsetof(CameraData, ZoomLevel)), EngineMetaFlag::Public);
	AddMember(Data, "ViewportX", LuaMember("float", offsetof(CameraData, ViewportX)), EngineMetaFlag::Public);
	AddMember(Data, "ViewportY", LuaMember("float", offsetof(CameraData, ViewportY)), EngineMetaFlag::Public);
	AddMember(Data, "ViewportWidth", LuaMember("float", offsetof(CameraData, ViewportWidth)), EngineMetaFlag::Public);
	AddMember(Data, "ViewportHeight", LuaMember("float", offsetof(CameraData, ViewportHeight)), EngineMetaFlag::Public);
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


	name = "Animation2D";
	Data = CreateLuaMetaClass(name + "Data", EngineMetaFlag::ComponentData);
	AddMember(Data, "thisID", LuaMember("unsigned int", offsetof(Animation2DData, thisID)), EngineMetaFlag::Private);
	Static = CreateLuaMetaStatic(name, EngineMetaFlag::Component);
	info_Add = GeGlobalMethodInfo(&Animation2D_Add);
	info_Add.memberName.push_back("ID");
	info_Get = GeGlobalMethodInfo(&Animation2D_Get);
	info_Get.memberName.push_back("ID");
	info_Has = GeGlobalMethodInfo(&Animation2D_Has);
	info_Has.memberName.push_back("ID");
	AddGlobalMethod(Static, name + "_Add", info_Add, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Get", info_Get, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Has", info_Has, EngineMetaFlag::Component);

	auto info_Play = GeGlobalMethodInfo(&Animation2D_Play);
	info_Play.memberName.push_back("ID");
	info_Play.memberName.push_back("AnimationIndex");
	auto info_Stop = GeGlobalMethodInfo(&Animation2D_Stop);
	info_Stop.memberName.push_back("ID");
	auto info_Reset = GeGlobalMethodInfo(&Animation2D_Reset);
	info_Reset.memberName.push_back("ID");
	auto info_Create = GeGlobalMethodInfo(&Animation2D_Create);
	info_Create.memberName.push_back("ID");
	info_Create.memberName.push_back("TextureName");
	info_Create.memberName.push_back("MaxFrameX");
	info_Create.memberName.push_back("MaxFrameY");
	info_Create.memberName.push_back("Speed");
	AddGlobalMethod(Static, name + "_Play", info_Play, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Stop", info_Stop, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Reset", info_Reset, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Create", info_Create, EngineMetaFlag::Component);
	auto info_GetFrameIndex = GeGlobalMethodInfo(&Animation2D_GetFrameIndex);
	info_GetFrameIndex.memberName.push_back("ID");
	info_GetFrameIndex.memberName.push_back("AnimationIndex");
	AddGlobalMethod(Static, "Animation2D_GetFrameIndex", info_GetFrameIndex, EngineMetaFlag::Component);



	name = "Rigidbody2D";
	Data = CreateLuaMetaClass(name + "Data", EngineMetaFlag::ComponentData);
	AddMember(Data, "thisID", LuaMember("unsigned int", offsetof(Rigidbody2DData, thisID)), EngineMetaFlag::Private);
	Static = CreateLuaMetaStatic(name, EngineMetaFlag::Component);
	info_Add = GeGlobalMethodInfo(&Rigidbody2D_Add);
	info_Add.memberName.push_back("ID");
	info_Get = GeGlobalMethodInfo(&Rigidbody2D_Get);
	info_Get.memberName.push_back("ID");
	info_Has = GeGlobalMethodInfo(&Rigidbody2D_Has);
	info_Has.memberName.push_back("ID");
	AddGlobalMethod(Static, name + "_Add", info_Add, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Get", info_Get, EngineMetaFlag::Component);
	AddGlobalMethod(Static, name + "_Has", info_Has, EngineMetaFlag::Component);

	auto info_CreateBoxCollider = GeGlobalMethodInfo(&Rigidbody2D_CreateBoxCollider);
	info_CreateBoxCollider.memberName.push_back("ID");
	info_CreateBoxCollider.memberName.push_back("Size");
	info_CreateBoxCollider.memberName.push_back("Center");
	AddGlobalMethod(Static, "Rigidbody2D_CreateBoxCollider", info_CreateBoxCollider, EngineMetaFlag::Component);

	auto info_CreateCircleCollider = GeGlobalMethodInfo(&Rigidbody2D_CreateCircleCollider);
	info_CreateCircleCollider.memberName.push_back("ID");
	info_CreateCircleCollider.memberName.push_back("Radius");
	info_CreateCircleCollider.memberName.push_back("Center");
	AddGlobalMethod(Static, "Rigidbody2D_CreateCircleCollider", info_CreateCircleCollider, EngineMetaFlag::Component);


	auto info_SetMotionType = GeGlobalMethodInfo(&Rigidbody2D_SetMotionType);
	info_SetMotionType.memberName.push_back("ID");
	info_SetMotionType.memberName.push_back("MotionType");
	AddGlobalMethod(Static, "Rigidbody2D_SetMotionType", info_SetMotionType, EngineMetaFlag::Component);
	auto info_SetPosition = GeGlobalMethodInfo(&Rigidbody2D_SetPosition);
	info_SetPosition.memberName.push_back("ID");
	info_SetPosition.memberName.push_back("position");
	AddGlobalMethod(Static, "Rigidbody2D_SetPosition", info_SetPosition, EngineMetaFlag::Component);
	auto info_SetRotation = GeGlobalMethodInfo(&Rigidbody2D_SetRotation);
	info_SetRotation.memberName.push_back("ID");
	info_SetRotation.memberName.push_back("rotation");
	AddGlobalMethod(Static, "Rigidbody2D_SetRotation", info_SetRotation, EngineMetaFlag::Component);
	auto info_SetVelocity = GeGlobalMethodInfo(&Rigidbody2D_SetVelocity);
	info_SetVelocity.memberName.push_back("ID");
	info_SetVelocity.memberName.push_back("velocity");
	AddGlobalMethod(Static, "Rigidbody2D_SetVelocity", info_SetVelocity, EngineMetaFlag::Component);
	auto info_SetSensor = GeGlobalMethodInfo(&Rigidbody2D_SetSensor);
	info_SetSensor.memberName.push_back("ID");
	info_SetSensor.memberName.push_back("Sensor");
	AddGlobalMethod(Static, "Rigidbody2D_SetSensor", info_SetSensor, EngineMetaFlag::Component);
	auto info_SetActive = GeGlobalMethodInfo(&Rigidbody2D_SetActive);
	info_SetActive.memberName.push_back("ID");
	info_SetActive.memberName.push_back("Active");
	AddGlobalMethod(Static, "Rigidbody2D_SetActive", info_SetActive, EngineMetaFlag::Component);
	auto info_SetLayer = GeGlobalMethodInfo(&Rigidbody2D_SetLayer);
	info_SetLayer.memberName.push_back("ID");
	info_SetLayer.memberName.push_back("LayerName");
	AddGlobalMethod(Static, "Rigidbody2D_SetLayer", info_SetLayer, EngineMetaFlag::Component);

}
BindManager::BindManager()
{

}

BindManager::~BindManager()
{

}

void BindManager::Initialize()
{
	PClass* GlobalVector2Data = CreateLuaMetaClass("PVector2", EngineMetaFlag::TypeClassData);
	AddMember(GlobalVector2Data, "x", LuaMember("float", offsetof(PVector2, x)), EngineMetaFlag::TypeClassData);
	AddMember(GlobalVector2Data, "y", LuaMember("float", offsetof(PVector2, y)), EngineMetaFlag::TypeClassData);
	PStatic* GlobalVector2 = CreateLuaMetaStatic("PVector2Function", EngineMetaFlag::TypeClass);
	auto v2_Create = GeGlobalMethodInfo(&PVector2_Create);
	v2_Create.memberName.push_back("x");
	v2_Create.memberName.push_back("y");
	AddGlobalMethod(GlobalVector2, "PVector2_Create", v2_Create, EngineMetaFlag::TypeClass);


	PClass* GlobalVector3Data = CreateLuaMetaClass("PVector3", EngineMetaFlag::TypeClassData);
	AddMember(GlobalVector3Data, "x", LuaMember("float", offsetof(PVector3, x)), EngineMetaFlag::TypeClassData);
	AddMember(GlobalVector3Data, "y", LuaMember("float", offsetof(PVector3, y)), EngineMetaFlag::TypeClassData);
	AddMember(GlobalVector3Data, "z", LuaMember("float", offsetof(PVector3, z)), EngineMetaFlag::TypeClassData);
	PStatic* GlobalVector3 = CreateLuaMetaStatic("PVector3Function", EngineMetaFlag::TypeClass);
	auto info_Normalize = GeGlobalMethodInfo(&PVector3_Normalize);
	info_Normalize.memberName.push_back("normalizeVector3");
	AddGlobalMethod(GlobalVector3, "PVector3_Normalize", info_Normalize, EngineMetaFlag::TypeClass);

	auto info_Length = GeGlobalMethodInfo(&PVector3_Length);
	info_Length.memberName.push_back("vector3");
	AddGlobalMethod(GlobalVector3, "PVector3_Length", info_Length, EngineMetaFlag::TypeClass);

	auto info_Distance = GeGlobalMethodInfo(&PVector3_Distance);
	info_Distance.memberName.push_back("from");
	info_Distance.memberName.push_back("to");
	AddGlobalMethod(GlobalVector3, "PVector3_Distance", info_Distance, EngineMetaFlag::TypeClass);

	auto info_Dot = GeGlobalMethodInfo(&PVector3_Dot);
	info_Dot.memberName.push_back("lhs");
	info_Dot.memberName.push_back("rhs");
	AddGlobalMethod(GlobalVector3, "PVector3_Dot", info_Dot, EngineMetaFlag::TypeClass);

	auto info_Cross = GeGlobalMethodInfo(&PVector3_Cross);
	info_Cross.memberName.push_back("lhs");
	info_Cross.memberName.push_back("rhs");
	AddGlobalMethod(GlobalVector3, "PVector3_Cross", info_Cross, EngineMetaFlag::TypeClass);

	auto info_Direction = GeGlobalMethodInfo(&PVector3_Direction);
	info_Direction.memberName.push_back("from");
	info_Direction.memberName.push_back("to");
	AddGlobalMethod(GlobalVector3, "PVector3_Direction", info_Direction, EngineMetaFlag::TypeClass);

	auto info_Lerp = GeGlobalMethodInfo(&PVector3_Lerp);
	info_Lerp.memberName.push_back("from");
	info_Lerp.memberName.push_back("to");
	info_Lerp.memberName.push_back("amount");
	AddGlobalMethod(GlobalVector3, "PVector3_Lerp", info_Lerp, EngineMetaFlag::TypeClass);

	auto info_ApproximatelyEquals = GeGlobalMethodInfo(&PVector3_ApproximatelyEquals);
	info_ApproximatelyEquals.memberName.push_back("lhs");
	info_ApproximatelyEquals.memberName.push_back("rhs");
	info_ApproximatelyEquals.memberName.push_back("epsilon");
	AddGlobalMethod(GlobalVector3, "PVector3_ApproximatelyEquals", info_ApproximatelyEquals, EngineMetaFlag::TypeClass);

	auto v3_Create = GeGlobalMethodInfo(&PVector3_Create);
	v3_Create.memberName.push_back("x");
	v3_Create.memberName.push_back("y");
	v3_Create.memberName.push_back("z");
	AddGlobalMethod(GlobalVector3, "PVector3_Create", v3_Create, EngineMetaFlag::TypeClass);

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


	//Pool
	PStatic* globalPool = CreateLuaMetaStatic("Pool", EngineMetaFlag::Class);
	auto PoolActive = GeGlobalMethodInfo(&Pool_Active);
	PoolActive.memberName.push_back("ScriptName");
	AddGlobalMethod(globalPool, "Pool_Active", PoolActive, EngineMetaFlag::Class);
	auto PoolDisable = GeGlobalMethodInfo(&Pool_Disable);
	PoolDisable.memberName.push_back("ScriptName");
	PoolDisable.memberName.push_back("ID");
	AddGlobalMethod(globalPool, "Pool_Disable", PoolDisable, EngineMetaFlag::Class);
	auto PoolClear = GeGlobalMethodInfo(&Pool_Clear);
	PoolClear.memberName.push_back("ScriptName");
	AddGlobalMethod(globalPool, "Pool_Clear", PoolClear, EngineMetaFlag::Class);
	auto PoolGetActiveCount = GeGlobalMethodInfo(&Pool_GetActiveCount);
	PoolGetActiveCount.memberName.push_back("ScriptName");
	AddGlobalMethod(globalPool, "Pool_GetActiveCount", PoolGetActiveCount, EngineMetaFlag::Class);
	auto PoolGetActiveID = GeGlobalMethodInfo(&Pool_GetActiveID);
	PoolGetActiveID.memberName.push_back("ScriptName");
	PoolGetActiveID.memberName.push_back("Index");
	AddGlobalMethod(globalPool, "Pool_GetActiveID", PoolGetActiveID, EngineMetaFlag::Class);
	auto PoolSetExpansionSize = GeGlobalMethodInfo(&Pool_SetExpansionSize);
	PoolSetExpansionSize.memberName.push_back("ScriptName");
	PoolSetExpansionSize.memberName.push_back("CreateCount");
	AddGlobalMethod(globalPool, "Pool_SetExpansionSize", PoolSetExpansionSize, EngineMetaFlag::Class);

	//Group
	PStatic* globalGroup = CreateLuaMetaStatic("Group", EngineMetaFlag::Class);
	auto CreateGroupAndEntity = GeGlobalMethodInfo(&Group_CreateGroupAndEntity);
	CreateGroupAndEntity.memberName.push_back("GroupName");
	CreateGroupAndEntity.memberName.push_back("ScriptName");
	AddGlobalMethod(globalGroup, "Group_CreateGroupAndEntity", CreateGroupAndEntity, EngineMetaFlag::Class);
	auto GroupSet = GeGlobalMethodInfo(&Group_Set);
	GroupSet.memberName.push_back("GroupName");
	GroupSet.memberName.push_back("ID");
	AddGlobalMethod(globalGroup, "Group_Set", GroupSet, EngineMetaFlag::Class);
	auto GroupRemove = GeGlobalMethodInfo(&Group_Remove);
	GroupRemove.memberName.push_back("GroupName");
	GroupRemove.memberName.push_back("ID");
	AddGlobalMethod(globalGroup, "Group_Remove", GroupRemove, EngineMetaFlag::Class);
	auto GroupClear = GeGlobalMethodInfo(&Group_Clear);
	GroupClear.memberName.push_back("GroupName");
	AddGlobalMethod(globalGroup, "Group_Clear", GroupClear, EngineMetaFlag::Class);
	auto GroupCount = GeGlobalMethodInfo(&Group_Count);
	GroupCount.memberName.push_back("GroupName");
	AddGlobalMethod(globalGroup, "Group_Count", GroupCount, EngineMetaFlag::Class);
	auto GroupFirst = GeGlobalMethodInfo(&Group_First);
	GroupFirst.memberName.push_back("GroupName");
	AddGlobalMethod(globalGroup, "Group_First", GroupFirst, EngineMetaFlag::Class);
	auto GroupGetCount = GeGlobalMethodInfo(&Group_GetCount);
	GroupGetCount.memberName.push_back("GroupName");
	AddGlobalMethod(globalGroup, "Group_GetCount", GroupGetCount, EngineMetaFlag::Class);
	auto GroupGetID = GeGlobalMethodInfo(&Group_GetID);
	GroupGetID.memberName.push_back("GroupName");
	GroupGetID.memberName.push_back("Index");
	AddGlobalMethod(globalGroup, "Group_GetID", GroupGetID, EngineMetaFlag::Class);



	RegisterComponentData();
	BindEnum();
}

void BindManager::Update()
{

}

void BindManager::Clear()
{}

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

	globalEnum = CreateNewEnum("MotionType");
	AddEnum(globalEnum, "Static");
	AddEnum(globalEnum, "Kinematic");
	AddEnum(globalEnum, "Dynamic");

	//globalEnum = CreateNewEnum("ColliderType");
	//AddEnum(globalEnum, "Box2D");
	//AddEnum(globalEnum, "Circle2D");

}
