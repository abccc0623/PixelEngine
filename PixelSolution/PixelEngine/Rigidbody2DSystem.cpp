#include "pch.h"
#include "Rigidbody2DSystem.h"
#include "PixelGraphicsAPI.h"
#include "Registry.h"
#include "Rigidbody2D.h"
#include "Collider2D.h"
#include "BoxCollider2D.h"
#include "CircleCollider2D.h"
#include "PixelEngine.h"
#include "PhysManager.h"
#include "PixelEngineAPI.h"
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
					if (box)
					{
						collider.type = ECS::Collider2D::Collider2DType::BOX;
						collider.BoxOffset = box->Offset;
						collider.Center = box->Center;
					}
				}
				else if (registry->Has<ECS::CircleCollider2D::CircleCollider2DData>(id))
				{
					auto circle = registry->Get<ECS::CircleCollider2D::CircleCollider2DData>(id);
					if (circle)
					{
						collider.type = ECS::Collider2D::Collider2DType::Circle;
						collider.CircleRadius = circle->Radius;
						collider.Center = circle->Center;
					}
				}

				auto shape = phys->CreateCollider(&collider);
				auto bodyID = phys->CreateRigidbody(data, shape, id);
				data->bodyID = bodyID.GetIndexAndSequenceNumber();
				data->IsCreate = true;
			}



			if (data->impulse != Pixel::Vector3(0, 0, 0))
			{
				phys->AddImpulse(JPH::BodyID(data->bodyID), data->impulse.x, data->impulse.y, data->impulse.z);
				data->impulse = Pixel::Vector3(0, 0, 0);
			}
			if (data->force != Pixel::Vector3(0, 0, 0))
			{
				phys->AddForce(JPH::BodyID(data->bodyID), data->force.x, data->force.y, data->force.z);
				data->force = Pixel::Vector3(0, 0, 0);
			}
			if (data->velocity != data->velocityCopy)
			{
				Pixel::Vector3 final = phys->GetVelocity(JPH::BodyID(data->bodyID));
				if (data->velocity.x != data->velocityCopy.x) final.x = data->velocity.x;
				if (data->velocity.y != data->velocityCopy.y) final.y = data->velocity.y;
				if (data->velocity.z != data->velocityCopy.z) final.z = data->velocity.z;

				phys->SetVelocity(JPH::BodyID(data->bodyID), final.x, final.y, final.z);

				data->velocity = final;
				data->velocityCopy = final;
			}
			phys->SyncPhysics(JPH::BodyID(data->bodyID));
		});
}

void ECS::Rigidbody2DSystem::Release()
{

}
