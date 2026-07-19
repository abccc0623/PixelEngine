#include "pch.h"
#include "ColliderFactory.h"
#include "Type/GlobalEnum.h"
#include "PVector2.h"

std::unordered_map<size_t, JPH::ShapeRefC> ColliderFactory::colliderMap;
ColliderFactory::ColliderFactory()
{
	colliderMap = std::unordered_map<size_t, JPH::ShapeRefC>();
}

ColliderFactory::~ColliderFactory()
{
	colliderMap.clear();
}

JPH::ShapeRefC ColliderFactory::CreateBox2D(PVector2* size, PVector2* center)
{
	size_t  key = GetColliderKey(size, center);
	auto find = colliderMap.find(key);
	if (find == colliderMap.end())
	{
		float halfX = size->x * 0.5f;
		float halfY = size->y * 0.5f;
		float halfZ = 0.05f;
		JPH::Ref<JPH::BoxShapeSettings> box = new JPH::BoxShapeSettings(JPH::Vec3(halfX, halfY, halfZ));
		box->mConvexRadius = std::min({ 0.05f,halfX * 0.5f,halfY * 0.5f,halfZ * 0.5f });

		JPH::Vec3 JPHcenter = JPH::Vec3(center->x, center->y, 0.0f);
		JPH::Quat JPHrot = JPH::Quat::sIdentity();

		JPH::Ref<JPH::RotatedTranslatedShapeSettings> offsetShape = new JPH::RotatedTranslatedShapeSettings(JPHcenter, JPHrot, box);
		JPH::ShapeSettings::ShapeResult result = offsetShape->Create();

		if (!result.IsValid()) PixelLog::Info(result.GetError().c_str());
		auto shape = result.Get();
		colliderMap.insert({ key, shape });
		return shape;
	}
	else
	{
		return find->second;
	}
}

JPH::ShapeRefC ColliderFactory::CreateCircle2D(float radius, PVector2* center)
{
	size_t key = GetColliderKey(radius, center);
	auto find = colliderMap.find(key);
	if (find == colliderMap.end())
	{
		JPH::Ref<JPH::SphereShapeSettings> circle = new JPH::SphereShapeSettings(radius);

		JPH::Vec3 JPHcenter = JPH::Vec3(center->x, center->y, 0.0f);
		JPH::Quat JPHrot = JPH::Quat::sIdentity();

		JPH::Ref<JPH::RotatedTranslatedShapeSettings> offsetShape =
			new JPH::RotatedTranslatedShapeSettings(JPHcenter, JPHrot, circle);

		JPH::ShapeSettings::ShapeResult result = offsetShape->Create();

		if (!result.IsValid()) PixelLog::Info(result.GetError().c_str());

		auto shape = result.Get();
		colliderMap.insert({ key, shape });
		return shape;
	}
	else
	{
		return find->second;
	}
}






size_t  ColliderFactory::GetColliderKey(PVector2* size, PVector2* center)
{
	size_t h = 0;
	auto combine = [&h](float value)
		{
			size_t v = std::hash<float>{}(value);
			h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2);
		};
	combine(center->x);
	combine(center->y);
	combine(size->x);
	combine(size->y);
	return h;
}

size_t ColliderFactory::GetColliderKey(float radius, PVector2* center)
{
	size_t h = 0;
	auto combine = [&h](float value)
		{
			size_t v = std::hash<float>{}(value);
			h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2);
		};
	combine(center->x);
	combine(center->y);
	combine(radius);
	return h;
}


//JPH::ShapeRefC ColliderFactory::CreateCircle2D(ECS::Collider2D::Collider2DData* collider)
//{
//	JPH::Ref<JPH::SphereShapeSettings> sphere = new JPH::SphereShapeSettings(collider->CircleRadius);
//
//	// 4. [최적화] 오프셋이 (0,0)인 경우 불필요한 데코레이터 패턴(RotatedTranslated) 생략
//	if (collider->Center.x == 0.0f && collider->Center.y == 0.0f)
//	{
//		JPH::ShapeSettings::ShapeResult result = sphere->Create();
//		if (!result.IsValid())
//		{
//			PixelLog::Error(result.GetError().c_str());
//			return nullptr;
//		}
//		return result.Get();
//	}
//
//	JPH::Ref<JPH::RotatedTranslatedShapeSettings> offsetShape =
//		new JPH::RotatedTranslatedShapeSettings(JPH::Vec3(collider->Center.x, collider->Center.y, 0.0f), JPH::Quat::sIdentity(), sphere);
//
//	JPH::ShapeSettings::ShapeResult result = offsetShape->Create();
//	if (!result.IsValid())
//	{
//		PixelLog::Error(result.GetError().c_str());
//		return nullptr;
//	}
//	return result.Get();
//}
