#include "pch.h"
#include "Rigidbody2DSystem.h"
#include "PixelGraphicsAPI.h"
#include "Registry.h"
#include "Rigidbody2D.h"
#include "PixelEngine.h"
#include "PhysManager.h"
#include "PixelEngineAPI.h"
#include "Physics2DData.h"
#include "ColliderFactory.h"
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
	auto& Chunked = registry->GetChunkedArray<Rigidbody2DData>();
	Chunked.ForEach([registry, phys](Rigidbody2DData* data, size_t index)
		{
			auto physData = registry->Get<Physics2DData>(data->thisID);
			if (physData == nullptr)
			{
				PixelLog::Error("[Rigidbody2D][Update] Missing Physics2D component. Entity ID: " + std::to_string(data->thisID));
				return;
			}
			if (physData->Create == false)
			{
				if (physData->colliderRefC == nullptr)
				{
					PVector2 size{ 1,1 };
					PVector2 center{ 0,0 };
					physData->colliderRefC = ColliderFactory::CreateBox2D(&size, &center);
					if (physData->colliderRefC == nullptr) return;
				}
				physData->bodyID = phys->CreateRigidbody(data, physData);
				if (physData->bodyID.IsInvalid()) return;
				physData->Create = true;
			}
			phys->SyncPhysics(physData->bodyID);
		});
}

void ECS::Rigidbody2DSystem::EditorUpdate(Registry* registry)
{
	auto& bodies = registry->GetChunkedArray<Rigidbody2DData>();
	bodies.ForEach([this, registry](Rigidbody2DData* data, size_t index)
		{
			phys->DebugDrawPreview(data, registry->Get<Physics2DData>(data->thisID));
		});
}
void ECS::Rigidbody2DSystem::Release()
{

}
