#include "pch.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
void* ECS::Transform::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<TransformData>(id);
	registry->AddComponent<WorldData>(id);
	TransformData* data = registry->Get<TransformData>(id);
	return registry->Get<TransformData>(id);
}
void* ECS::Transform::GetComponent(unsigned int id)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Transform][GetComponent] Not Find Component");
	}
	return data;
}
bool ECS::Transform::HasComponent(unsigned int id)
{
	auto registry = GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

std::string ECS::Transform::BindJit()
{
	std::string jit;
	jit += "\ttypedef struct { float x, y, z; } Vec3; \n";
	jit += "\ttypedef struct __attribute__((aligned(16))) \n";
	jit += "\t{ \n";
	jit += "\t\tVec3 pos; \n";
	jit += "\t\tuint32_t:32; \n";
	jit += "\t\tVec3 rot; \n";
	jit += "\t\tuint32_t: 32; \n";
	jit += "\t\tVec3 scale; \n";
	jit += "\t\tuint32_t: 32; \n";
	jit += "\t} TransformData;\n";
	return jit;
}
