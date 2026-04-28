#include "pch.h"
#include "BoxCollider2D.h"
#include "PixelEngine.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "PhysManager.h"
#include "PixelGraphicsAPI.h"
#include "RenderringData.h"
#include "BindManager.h"
#include "Module/Transform.h"
extern PixelEngine* Engine;
BoxCollider2D::BoxCollider2D():collision(nullptr)
{
	targetGravity = 1.0f;
}

BoxCollider2D::~BoxCollider2D()
{
}

void BoxCollider2D::Awake()
{
	if (collision == nullptr)
	{
		collision = Engine->GetFactory<PhysManager>();
	}
}

void BoxCollider2D::Start()
{
	
}

void BoxCollider2D::PhysicsUpdate()
{
	
	if (collision == nullptr)
	{
		collision = Engine->GetFactory<PhysManager>();
	}
	collision->SyncPhysics(targetBodyID);
}

void BoxCollider2D::SetOffset(float offsetX, float offsetY)
{
	this->offsetX = offsetX;
	this->offsetY = offsetY;
}

void BoxCollider2D::SetGravity(float gravity)
{
	targetGravity = gravity;
}

void BoxCollider2D::SetCreateActive(bool active)
{
	this->active = active;
}

void BoxCollider2D::SetPhysType(int type)
{
	motionType = type;
}

void BoxCollider2D::CreatePhys()
{
	CreateCollider();
	CreateBody();
}

void BoxCollider2D::CreateCollider()
{
	float halfX = offsetX * 0.5f;
	float halfY = offsetY * 0.5f;
	float halfZ = 0.1f;

	JPH::Ref<JPH::BoxShapeSettings> box = new JPH::BoxShapeSettings(JPH::Vec3(halfX, halfY, halfZ));
	box->mConvexRadius = 0.01f;

	if (centerX != 0.0f || centerY != 0.0f)
	{
		// ?? 여기가 핵심! Compound 대신 RotatedTranslatedShapeSettings를 사용합니다.
		// 이 녀석은 무게 중심은 그대로 두고, 껍데기만 우리가 원하는 곳으로 밀어줍니다.
		JPH::Ref<JPH::RotatedTranslatedShapeSettings> offsetShape =
			new JPH::RotatedTranslatedShapeSettings(JPH::Vec3(centerX, centerY, 0.0f), JPH::Quat::sIdentity(), box);

		JPH::ShapeSettings::ShapeResult result = offsetShape->Create();
		if (!result.IsValid())
		{
			PixelLog::Info(result.GetError().c_str());
		}
		targetRef = result.Get();
	}
	else
	{
		// 중심점이 0,0이면 그냥 기본 박스 사용
		JPH::ShapeSettings::ShapeResult boxResult = box->Create();
		if (!boxResult.IsValid())
		{
			PixelLog::Info(boxResult.GetError().c_str());
		}
		targetRef = boxResult.Get();
	}
}

void BoxCollider2D::CreateBody()
{
	if (collision == nullptr)
	{
		collision = Engine->GetFactory<PhysManager>();
	}
	auto tr = targetObject->GetTransform();
	float angleInRadians = tr->Rotation.Z;
	JPH::Quat q = JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), angleInRadians);
	JPH::RVec3 v = JPH::RVec3(tr->Position.X,tr->Position.Y, 0.0f);

	JPH::EMotionType type = JPH::EMotionType::Static;
	switch (motionType)
	{
	case 0:
		type = JPH::EMotionType::Static;
		break;
	case 1:
		type = JPH::EMotionType::Kinematic;
		break;
	case 2:
		type = JPH::EMotionType::Dynamic;
		break;
	}

	JPH::BodyCreationSettings settings(
		targetRef,
		v,
		q,
		type,
		type == JPH::EMotionType::Static ? 0 : 1
	);
	settings.mGravityFactor = targetGravity;
	settings.mAllowedDOFs = JPH::EAllowedDOFs::Plane2D;
	targetBodyID = collision->CreateBody2D(&settings, active,targetObject);
}

void BoxCollider2D::SetCenter(float x, float y)
{
	centerX = x;
	centerY = y;
}
