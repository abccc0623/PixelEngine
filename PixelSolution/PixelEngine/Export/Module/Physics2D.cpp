#include "pch.h"
#include "Physics2D.h"
#include "PixelEngine.h"
#include "PhysManager.h"
#include "PixelGraphicsAPI.h"
#include "RenderringData.h"
#include "BindManager.h"
#include "Type/GlobalEnum.h"

extern PixelEngine* Engine;
Physics2D::Physics2D() :collision(nullptr)
{
	colliderSetting.colliderType = 0;
	colliderSetting.Key = "KeyName";
	colliderSetting.box2D.CenterX = 0.0f;
	colliderSetting.box2D.CenterY = 0.0f;
	colliderSetting.box2D.OffsetX = 0.5f;
	colliderSetting.box2D.OffsetY = 0.5f;

	rigidbodySetting.Active = true;
	rigidbodySetting.AutoSleep = true;
	rigidbodySetting.Gravity = 1.0f;
	rigidbodySetting.Lock[0] = false;
	rigidbodySetting.Lock[1] = false;
	rigidbodySetting.Lock[2] = true;
	rigidbodySetting.Lock[3] = false;
	rigidbodySetting.Lock[4] = false;
	rigidbodySetting.Lock[5] = false;
}

Physics2D::~Physics2D()
{
}

void Physics2D::Awake()
{
	if (collision == nullptr)
	{
		collision = Engine->GetFactory<PhysManager>();
	}
}

void Physics2D::Start()
{

}

void Physics2D::PhysicsUpdate()
{
	if (collision == nullptr)
	{
		collision = Engine->GetFactory<PhysManager>();
	}
	if (targetRef.GetPtr() == nullptr)
	{
		//targetRef = collision->CreateCollider(colliderSetting);
	}
	if (targetBodyID.IsInvalid())
	{
		//targetBodyID = collision->CreateRigidbody(rigidbodySetting, targetRef, targetObject);
	}
	collision->SyncPhysics(targetBodyID);
}

void Physics2D::SetActive(bool active)
{
	collision->SetActive(targetBodyID, active);
}

bool Physics2D::GetActive()
{
	return collision->GetActive(targetBodyID);
}

void Physics2D::SetVelocity(float x, float y)
{
	if (!targetBodyID.IsInvalid())
	{
		collision->SetVelocity(targetBodyID, x, y, 0.0f);
	}
}

void Physics2D::SetVelocityX(float x)
{
	if (!targetBodyID.IsInvalid())
	{
		collision->SetVelocityX(targetBodyID, x);
	}
}

void Physics2D::SetVelocityY(float y)
{
	if (!targetBodyID.IsInvalid())
	{
		collision->SetVelocityY(targetBodyID, y);
	}
}

void Physics2D::SetCollider(sol::table table)
{
	if (collision == nullptr)
	{
		collision = Engine->GetFactory<PhysManager>();
	}

	sol::optional<std::string> isKey = table["Key"];
	if (isKey)
	{
		colliderSetting.Key = isKey.value();
	}
	sol::optional<ColliderType> type = table["ColliderType"];
	if (type)
	{
		//colliderSetting.colliderType = (int)type.value();
		//switch (colliderSetting.colliderType)
		//{
		//case (int)ColliderType::Box2D:
		//{
		//	colliderSetting.box2D.CenterX = 0.0f;
		//	colliderSetting.box2D.CenterY = 0.0f;
		//	sol::optional<sol::table> centerOption = table["Center"];
		//	if (centerOption)
		//	{
		//		sol::table centerTable = centerOption.value();
		//		colliderSetting.box2D.CenterX = centerTable["x"];
		//		colliderSetting.box2D.CenterY = centerTable["y"];
		//	}
		//	colliderSetting.box2D.OffsetX = 0.5f;
		//	colliderSetting.box2D.OffsetY = 0.5f;
		//	sol::optional<sol::table> offsetOption = table["Offset"];
		//	if (offsetOption)
		//	{
		//		sol::table offsetTable = offsetOption.value();
		//		colliderSetting.box2D.OffsetX = offsetTable["x"];
		//		colliderSetting.box2D.OffsetY = offsetTable["y"];
		//	}
		//}
		//break;
		//case (int)ColliderType::Circle2D:
		//{
		//	colliderSetting.circle2D.CenterX = 0.0f;
		//	colliderSetting.circle2D.CenterY = 0.0f;
		//	sol::optional<sol::table> centerOption = table["Center"];
		//	if (centerOption)
		//	{
		//		sol::table centerTable = centerOption.value();
		//		colliderSetting.circle2D.CenterX = centerTable["x"];
		//		colliderSetting.circle2D.CenterY = centerTable["y"];
		//	}
		//	colliderSetting.circle2D.Radius = 0.5f;
		//	sol::optional<float> Radius = table["Radius"];
		//	if (Radius)
		//	{
		//		colliderSetting.circle2D.Radius = Radius.value();
		//	}
		//	collision->CreateCollider(colliderSetting);
		//}
		//break;
		//}
		//targetRef = collision->CreateCollider(colliderSetting);
	}
}

void Physics2D::SetRigidbody(sol::table table)
{
	if (collision == nullptr)
	{
		collision = Engine->GetFactory<PhysManager>();
	}
	sol::optional<ColliderMotionType> isColliderMotionType = table["ColliderMotionType"];
	if (isColliderMotionType)
	{
		rigidbodySetting.MotionType = (int)isColliderMotionType.value();
	}
	sol::optional<bool> isActive = table["Active"];
	if (isActive)
	{
		rigidbodySetting.Active = isActive.value();
	}
	sol::optional<float> isGravity = table["Gravity"];
	if (isGravity)
	{
		rigidbodySetting.Gravity = isGravity.value();
	}
	sol::optional<bool> isAutoSleep = table["AutoSleep"];
	if (isActive)
	{
		rigidbodySetting.AutoSleep = isAutoSleep.value();
	}
	sol::optional<bool> isSensor = table["Sensor"];
	if (isSensor)
	{
		rigidbodySetting.Sensor = isSensor.value();
	}
	float requestedRestitution = table["Restitution"].get_or(0.0f);
	float safeRestitution = std::clamp(requestedRestitution, 0.0f, 1.0f);
	if (requestedRestitution > 1.0f)
	{
		PixelLog::Warn("루아에서 설정된 Restitution 값이 1.0을 초과했습니다. 엔진 보호를 위해 1.0으로 강제 조정됩니다.");
	}
	rigidbodySetting.Restitution = safeRestitution;
	float requestedFriction = table["Friction"].get_or(0.2f);
	float safeFriction = std::clamp(requestedFriction, 0.0f, 5.0f);
	rigidbodySetting.Friction = requestedFriction;

	float requestedDamping = table["LinearDamping"].get_or(0.05f);
	float safeDamping = std::clamp(requestedDamping, 0.0f, 10.0f);
	rigidbodySetting.LinearDamping = requestedDamping;

	sol::optional<bool> isLock[6];
	isLock[0] = table["LockPositionX"];
	isLock[1] = table["LockPositionY"];
	isLock[2] = table["LockPositionZ"];
	isLock[3] = table["LockRotationX"];
	isLock[4] = table["LockRotationY"];
	isLock[5] = table["LockRotationZ"];
	for (int i = 0; i < 6; i++)
	{
		if (isLock[i]) rigidbodySetting.Lock[i] = isLock[i].value();
	}
	if (targetRef.GetPtr() == nullptr)
	{
		//argetRef = collision->CreateCollider(colliderSetting);
		//PixelLog::Warn(targetObject->name + "정의된 Collider가 없어 기본 Collider로 변경합니다.");
	}
	//targetBodyID = collision->CreateRigidbody(rigidbodySetting, targetRef, targetObject);
}

void Physics2D::SetPosition(float x, float y, float z)
{
	//if (!targetBodyID.IsInvalid())
	//{
	//	collision->SetPosition(targetBodyID, x, y, 0.0f, rigidbodySetting.Active);
	//}
	//else
	//{
	//	transform->Position.X = x;
	//	transform->Position.Y = y;
	//	transform->Position.Z = z;
	//}
}

void Physics2D::SetRotation(float x, float y, float z)
{
	//if (!targetBodyID.IsInvalid())
	//{
	//	collision->SetRotation(targetBodyID, x, y, z, rigidbodySetting.Active);
	//}
	//else
	//{
	//	transform->Rotation.X = x;
	//	transform->Rotation.Y = y;
	//	transform->Rotation.Z = z;
	//}
}

void Physics2D::AddForce(float x, float y, float z)
{
	if (!targetBodyID.IsInvalid())
	{
		collision->AddForce(targetBodyID, x, y, z);
	}
}

void Physics2D::AddImpulse(float x, float y, float z)
{
	if (!targetBodyID.IsInvalid())
	{
		collision->AddImpulse(targetBodyID, x, y, z);
	}
}
