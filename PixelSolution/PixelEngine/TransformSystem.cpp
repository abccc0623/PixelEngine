#include "pch.h"
#include "TransformSystem.h"
#include "PixelEngineAPI.h"
#include "PTransform.h"
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
	auto& transformarray = registry->GetArray<ECS::Transform::TransformData>();
	auto& worldarray = registry->GetArray<ECS::Transform::WorldData>();


	int size = transformarray.size();
	for (int i = 0; i < size; i++)
	{
		glm::vec3 pos(transformarray[i].position.x, transformarray[i].position.y, transformarray[i].position.z);
		glm::quat rot(transformarray[i].rotation.w, transformarray[i].rotation.x, transformarray[i].rotation.y, transformarray[i].rotation.z);
		glm::vec3 scl(transformarray[i].scale.x, transformarray[i].scale.y, transformarray[i].scale.z);

		// 3. T(이동), R(회전), S(크기) 개별 행렬 생성
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 rotationMatrix = glm::mat4_cast(rot); 
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scl);

		glm::mat4 worldMatrix = translationMatrix * rotationMatrix * scaleMatrix;

		worldarray[i].world = worldMatrix;
		transformarray[i].bitmask &= ~0x01;
	}
}

void ECS::TransformSystem::Release()
{

}
