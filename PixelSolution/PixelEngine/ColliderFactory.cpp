#include "pch.h"
#include "ColliderFactory.h"
#include "Type/GlobalEnum.h"

ColliderFactory::ColliderFactory()
{
	colliderMap = std::unordered_map<JPH::Shape*, JPH::ShapeRefC>();
}

ColliderFactory::~ColliderFactory()
{
	colliderMap.clear();
}

void ColliderFactory::Clear()
{
	colliderMap.clear();
}


JPH::ShapeRefC ColliderFactory::CreateBox2D(ECS::Collider2D::Collider2DData* collider)
{
	float halfX = collider->BoxOffset.x;
	float halfY = collider->BoxOffset.y;
	float halfZ = 1.0f;
	JPH::Ref<JPH::BoxShapeSettings> box = new JPH::BoxShapeSettings(JPH::Vec3(halfX, halfY, halfZ));
	box->mConvexRadius = 0.05f;
	JPH::Ref<JPH::RotatedTranslatedShapeSettings> offsetShape =
		new JPH::RotatedTranslatedShapeSettings(JPH::Vec3(collider->Center.x, collider->Center.y, 0.0f), JPH::Quat::sIdentity(), box);
	JPH::ShapeSettings::ShapeResult result = offsetShape->Create();
	if (!result.IsValid()) PixelLog::Info(result.GetError().c_str());

	return result.Get();
}

JPH::ShapeRefC ColliderFactory::CreateCircle2D(ECS::Collider2D::Collider2DData* collider)
{
	JPH::Ref<JPH::SphereShapeSettings> sphere = new JPH::SphereShapeSettings(collider->CircleRadius);

	// 4. [최적화] 오프셋이 (0,0)인 경우 불필요한 데코레이터 패턴(RotatedTranslated) 생략
	if (collider->Center.x == 0.0f && collider->Center.y == 0.0f)
	{
		JPH::ShapeSettings::ShapeResult result = sphere->Create();
		if (!result.IsValid())
		{
			PixelLog::Error(result.GetError().c_str());
			return nullptr;
		}
		return result.Get();
	}

	JPH::Ref<JPH::RotatedTranslatedShapeSettings> offsetShape =
		new JPH::RotatedTranslatedShapeSettings(JPH::Vec3(collider->Center.x, collider->Center.y, 0.0f), JPH::Quat::sIdentity(), sphere);

	JPH::ShapeSettings::ShapeResult result = offsetShape->Create();
	if (!result.IsValid())
	{
		PixelLog::Error(result.GetError().c_str());
		return nullptr;
	}
	return result.Get();
}
