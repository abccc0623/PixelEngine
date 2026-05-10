#include "pch.h"
#include "PTransform.h"
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
}

void ECS::Transform::SetRotation(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	data->rotation = { x ,y,z,0 };
}

void ECS::Transform::SetScale(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	data->scale = { x ,y,z };
}
