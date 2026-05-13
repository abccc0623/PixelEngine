#include "pch.h"
#include "Rigidbody2DSystem.h"
#include "PixelGraphicsAPI.h"
#include "Registry.h"
#include "Rigidbody2D.h"
#include "Collider2D.h"
#include "PixelEngine.h"
#include "PhysManager.h"
extern PixelEngine* Engine;
ECS::Rigidbody2DSystem::Rigidbody2DSystem()
{
	phys = Engine->GetFactory<PhysManager>();
}
ECS::Rigidbody2DSystem::~Rigidbody2DSystem()
{

}

void ECS::Rigidbody2DSystem::Update(ECS::Registry* registry)
{
	auto& collider2DArray = registry->GetArray<ECS::Rigidbody2D::Rigidbody2DData>();
	int size = collider2DArray.size();
	for (int i = 0; i < size; i++)
	{
		auto id = registry->GetEntityID<ECS::Rigidbody2D::Rigidbody2DData>(i);
		auto data = registry->Get<Collider2D::Collider2DData>(id);
		if (data != nullptr)
		{
			//생성한다
			if (data->IsCreate == false && collider2DArray[i].IsCreate == false)
			{
				auto shape = phys->CreateCollider(data);
				auto bodyID = phys->CreateRigidbody(&collider2DArray[i], shape, id);
				collider2DArray[i].bodyID = bodyID.GetIndexAndSequenceNumber();
				data->IsChange = false;
				data->IsCreate = true;
				collider2DArray[i].IsCreate = true;
			}

			//생성되었고 변경도 된 애들
			if (collider2DArray[i].IsCreate == true && collider2DArray[i].IsChange == true)
			{
				collider2DArray[i].IsChange = false;
			}

			if (collider2DArray[i].IsCreate == true)
			{
				phys->SyncPhysics(JPH::BodyID(collider2DArray[i].bodyID));
			}
		}
	}
}

void ECS::Rigidbody2DSystem::Release()
{

}
