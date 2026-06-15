#include "pch.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
void* ECS::Transform::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<TransformData>(id);
	registry->Add<WorldData>(id);
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
	std::string jit = R"(
	---@class TransformData
	---@field pos Vector3
	---@field rot Vector3
	---@field scale Vector3
ffi.cdef[[
	typedef struct __attribute__((aligned(16))) 
	{ 
		Vector3 pos; 
		uint32_t: 32; 
		Vector3 rot; 
		uint32_t: 32; 
		Vector3 scale; 
		uint32_t: 32;
	} TransformData;
]]
    )";
	return jit;
}
