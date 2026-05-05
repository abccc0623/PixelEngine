#include "pch.h"
#include "ColliderFactory.h"
#include "Type/GlobalEnum.h"

ColliderFactory::ColliderFactory()
{
    colliderMap = std::unordered_map<std::string, JPH::ShapeRefC>();
}

ColliderFactory::~ColliderFactory()
{
    colliderMap.clear();
}

void ColliderFactory::Clear()
{
	colliderMap.clear();
}


JPH::ShapeRefC ColliderFactory::CreateBox2D(PhysCollider& collider)
{
    auto find = colliderMap.find({collider.Key});
    if (find != colliderMap.end())
    {
        return find->second;
    }
    float halfX = collider.box2D.OffsetX;
    float halfY = collider.box2D.OffsetY;
    float halfZ = 1.0f;
    JPH::Ref<JPH::BoxShapeSettings> box = new JPH::BoxShapeSettings(JPH::Vec3(halfX, halfY, halfZ));
    box->mConvexRadius = 0.05f;
    JPH::Ref<JPH::RotatedTranslatedShapeSettings> offsetShape =
        new JPH::RotatedTranslatedShapeSettings(JPH::Vec3(collider.box2D.CenterX, collider.box2D.CenterY, 0.0f), JPH::Quat::sIdentity(), box);
    JPH::ShapeSettings::ShapeResult result = offsetShape->Create();
    if (!result.IsValid()) PixelLog::Info(result.GetError().c_str());
    colliderMap.insert({ collider.Key,result.Get() });
    return result.Get();
}

JPH::ShapeRefC ColliderFactory::CreateCircle2D(PhysCollider& collider)
{
    auto find = colliderMap.find({ collider.Key });
    if (find != colliderMap.end())
    {
        return find->second;
    }
    // 3. Jolt Sphere Shape 생성 (2D 환경의 Circle 역할 수행)
    JPH::Ref<JPH::SphereShapeSettings> sphere = new JPH::SphereShapeSettings(collider.circle2D.Radius);

    // 4. [최적화] 오프셋이 (0,0)인 경우 불필요한 데코레이터 패턴(RotatedTranslated) 생략
    if (collider.circle2D.CenterX == 0.0f && collider.circle2D.CenterY == 0.0f)
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
        new JPH::RotatedTranslatedShapeSettings(JPH::Vec3(collider.circle2D.CenterX, collider.circle2D.CenterY, 0.0f), JPH::Quat::sIdentity(), sphere);

    JPH::ShapeSettings::ShapeResult result = offsetShape->Create();
    if (!result.IsValid())
    {
        PixelLog::Error(result.GetError().c_str());
        return nullptr;
    }
    colliderMap.insert({ collider.Key,result.Get() });
    return result.Get();
}
