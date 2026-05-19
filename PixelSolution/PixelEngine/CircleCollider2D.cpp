#include "pch.h"
#include "CircleCollider2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
void* ECS::CircleCollider2D::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<CircleCollider2DData>(id);
	return registry->Get<CircleCollider2DData>(id);
}

void* ECS::CircleCollider2D::GetComponent(unsigned int id)
{
	auto registry = GetRegistry();
	CircleCollider2DData* data = registry->Get<CircleCollider2DData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[CircleCollider2D][GetComponent] Not Find Component");
	}
	return data;
}

bool ECS::CircleCollider2D::HasComponent(unsigned int id)
{
	auto registry = GetRegistry();
	CircleCollider2DData* data = registry->Get<CircleCollider2DData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

std::string ECS::CircleCollider2D::BindJit()
{
	std::string jit = R"(
	---@class CircleCollider2DData
	---@field center Vector2
	---@field radius number
ffi.cdef[[
    typedef struct 
    { 
        Vector2 center; 
		float radius;
    } CircleCollider2DData;
]]
    )";
	return jit;
}
