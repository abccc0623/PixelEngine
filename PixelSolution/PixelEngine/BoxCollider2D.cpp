#include "pch.h"
#include "BoxCollider2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
void* ECS::BoxCollider2D::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<BoxCollider2DData>(id);
	return registry->Get<BoxCollider2DData>(id);
}

void* ECS::BoxCollider2D::GetComponent(unsigned int id)
{
	auto registry = GetRegistry();
	BoxCollider2DData* data = registry->Get<BoxCollider2DData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[BoxCollider2D][GetComponent] Not Find Component");
	}
	return data;
}

bool ECS::BoxCollider2D::HasComponent(unsigned int id)
{
	auto registry = GetRegistry();
	BoxCollider2DData* data = registry->Get<BoxCollider2DData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

std::string ECS::BoxCollider2D::BindJit()
{
	std::string jit = R"(
	---@class BoxCollider2DData
	---@field center Vector2
	---@field offset Vector2
ffi.cdef[[
    typedef struct 
    { 
        Vector2 center; 
		Vector2 offset;
    } BoxCollider2DData;
]]
    )";
	return jit;
}
