#include "pch.h"
#include "Collider2DSystem.h"
#include "PixelGraphicsAPI.h"
#include "Registry.h"
#include "Collider2D.h"
#include "PhysManager.h"
#include "PixelEngine.h"
extern PixelEngine* Engine;
ECS::Collider2DSystem::Collider2DSystem()
{
	phys = Engine->GetFactory<PhysManager>();
}

ECS::Collider2DSystem::~Collider2DSystem()
{

}

void ECS::Collider2DSystem::Update(Registry* registry)
{
	auto& collider2DArray = registry->GetArray<ECS::Collider2D::Collider2DData>();
	int size = collider2DArray.size();
	for (int i = 0; i < size; i++)
	{
		auto id = registry->GetEntityID<ECS::Collider2D::Collider2DData>(i);
		auto data = registry->Get<ECS::Collider2D::Collider2DData>(id);
		if (data != nullptr)
		{
			//생성한다
			if (data->IsCreate == false)
			{
				phys->CreateCollider(data);

				data->IsChange = false;
				data->IsCreate = true;
			}

			//생성되었고 변경도 된 애들
			if (data->IsCreate == true && data->IsChange == true)
			{
				data->IsChange = false;
			}
		}
	}
}

void ECS::Collider2DSystem::Release()
{

}
