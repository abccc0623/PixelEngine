#include "pch.h"
#include "TransformSystem.h"
#include "PixelEngineAPI.h"
#include "Transform.h"
#include "Registry.h"
#include "PixelMeta.h"

ECS::TransformSystem::TransformSystem()
{

}

ECS::TransformSystem::~TransformSystem()
{

}

void ECS::TransformSystem::Update(ECS::Registry* registry)
{
	auto& T = registry->GetChunkedArray<ECS::Transform::TransformData>();
	T.ForEach([registry](ECS::Transform::TransformData* data, size_t index)
		{
			glm::vec3 eulerAngles(data->rotation.x, data->rotation.y, data->rotation.z);

			glm::vec3 pos(data->position.x, data->position.y, data->position.z);
			glm::quat rot(eulerAngles);
			glm::vec3 scl(data->scale.x, data->scale.y, data->scale.z);

			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), pos);
			glm::mat4 rotationMatrix = glm::mat4_cast(rot);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scl);

			glm::mat4 worldMatrix = translationMatrix * rotationMatrix * scaleMatrix;

			auto id = registry->GetEntityID<ECS::Transform::TransformData>(index);
			auto world = registry->Get<ECS::Transform::WorldData>(id);
			if (world != nullptr)
			{
				world->world = worldMatrix;
				data->bitmask &= ~0x01;
			}
		});
}

void ECS::TransformSystem::EditorUpdate(Registry* registry)
{
	Update(registry);
}

void ECS::TransformSystem::Release()
{

}
