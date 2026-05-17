#include "pch.h"
#include "Rigidbody2DSystem.h"
#include "PixelGraphicsAPI.h"
#include "Registry.h"
#include "Rigidbody2D.h"
#include "Collider2D.h"
#include "BoxCollider2D.h"
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
	auto phys = Engine->GetFactory<PhysManager>();
	auto& Chunked = registry->GetChunkedArray<ECS::Rigidbody2D::Rigidbody2DData>();
	Chunked.ForEach([registry, phys](ECS::Rigidbody2D::Rigidbody2DData* data, size_t index)
		{
			if (data->IsCreate == false)
			{
				ECS::Collider2D::Collider2DData collider;
				auto id = registry->GetEntityID<ECS::Rigidbody2D::Rigidbody2DData>(index);
				if (registry->Has<ECS::BoxCollider2D::BoxCollider2DData>(id))
				{
					auto box = registry->Get<ECS::BoxCollider2D::BoxCollider2DData>(id);
					collider.type = ECS::Collider2D::Collider2DType::BOX;
					collider.BoxOffset = box->Offset;
					collider.Center = box->Center;
				}

				auto shape = phys->CreateCollider(&collider);
				auto bodyID = phys->CreateRigidbody(data, shape, id);
				data->bodyID = bodyID.GetIndexAndSequenceNumber();
				data->IsCreate = true;
			}
			phys->SetVelocity(JPH::BodyID(data->bodyID), data->velocity.x, data->velocity.y, data->velocity.z);
			phys->SyncPhysics(JPH::BodyID(data->bodyID));
		});
}

void ECS::Rigidbody2DSystem::Release()
{

}
