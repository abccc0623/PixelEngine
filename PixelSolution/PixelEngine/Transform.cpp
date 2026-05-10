#include "pch.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
void ECS::Transform::Add(unsigned int id)
{
	auto registry = GetRegistry();

	registry->AddComponent<TransformData>(id);
	registry->AddComponent<WorldData>(id);
}
void ECS::Transform::SetPosition(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	data->position = { x,y,z };
	data->bitmask |= 0x01;
}

void ECS::Transform::SetRotation(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);

	glm::vec3 eulerAngles(x, y, z);
	glm::quat newRot(eulerAngles);
	data->rotation.x = newRot.x;
	data->rotation.y = newRot.y;
	data->rotation.z = newRot.z;
	data->rotation.w = newRot.w;
	data->bitmask |= 0x01;
}

void ECS::Transform::SetScale(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	data->scale = { x ,y,z };
	data->bitmask |= 0x01;
}

void ECS::Transform::AddPosition(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	data->position.x += x;
	data->position.y += y;
	data->position.z += z;
	data->bitmask |= 0x01;
}

void ECS::Transform::AddRotation(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);

	glm::quat currentRot(data->rotation.w, data->rotation.x, data->rotation.y, data->rotation.z);
	glm::vec3 deltaEuler(x, y, z);
	glm::quat deltaRot(deltaEuler);
	currentRot = currentRot * deltaRot;
	data->rotation.x = currentRot.x;
	data->rotation.y = currentRot.y;
	data->rotation.z = currentRot.z;
	data->rotation.w = currentRot.w;
	data->bitmask |= 0x01;
}

void ECS::Transform::AddScale(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	data->scale.x += x;
	data->scale.y += y;
	data->scale.z += z;
	data->bitmask |= 0x01;
}
