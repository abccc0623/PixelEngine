#include "pch.h"
#include "TransformSystem.h"
#include "PixelEngineAPI.h"
#include "Transform.h"
#include "Registry.h"
#include "PixelMeta.h"
#include "EntityObject.h"

namespace
{
	glm::mat4 MakeLocalMatrix(const TransformData& transform)
	{
		const glm::vec3 position(transform.position.x, transform.position.y, transform.position.z);
		const glm::vec3 eulerAngles(transform.rotation.x, transform.rotation.y, transform.rotation.z);
		const glm::vec3 scale(transform.scale.x, transform.scale.y, transform.scale.z);

		const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 rotationMatrix = glm::mat4_cast(glm::quat(eulerAngles));
		const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
		return translationMatrix * rotationMatrix * scaleMatrix;
	}

	void TransformUpdate(unsigned int id, ECS::Registry* registry, const glm::mat4& parentWorld)
	{
		auto* entity = FindEntity(id);
		auto* transform = registry->Get<TransformData>(id);
		auto* world = registry->Get<WorldData>(id);
		if (entity == nullptr || transform == nullptr || world == nullptr)
		{
			return;
		}

		const glm::mat4 localMatrix = MakeLocalMatrix(*transform);
		const glm::mat4 worldMatrix = parentWorld * localMatrix;
		world->local = localMatrix;
		world->world = worldMatrix;
		transform->bitmask &= ~0x01;

		for (const unsigned int childID : entity->GetChild())
		{
			TransformUpdate(childID, registry, worldMatrix);
		}
	}
}

ECS::TransformSystem::TransformSystem()
{

}

ECS::TransformSystem::~TransformSystem()
{

}

void ECS::TransformSystem::Update(ECS::Registry* registry)
{
	auto& T = registry->GetChunkedArray<TransformData>();
	T.ForEach([registry](TransformData* data, size_t index)
		{
			auto thisEntity = FindEntity(data->thisID);
			if (thisEntity != nullptr && thisEntity->GetParentID() == -1)
			{
				TransformUpdate(data->thisID, registry, glm::mat4(1.0f));
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
