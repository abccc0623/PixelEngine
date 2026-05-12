#include "pch.h"
#include "Collider2D.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
void ECS::Collider2D::Add(unsigned int id)
{
	auto registry = GetRegistry();
	registry->AddComponent<Collider2DData>(id);
	auto data = registry->Get<Collider2DData>(id);
}

void ECS::Collider2D::SetBoxOffset(unsigned int id, float x, float y)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Collider2DData>(id);
	if (data != nullptr)
	{
		data->BoxOffset = { x ,y };
		data->type = Collider2DType::BOX;
		data->IsChange = true;
	}
	else
	{
		PixelLog::Error("[Collider2D][SetBoxOffset] Not Find Component");
	}
}

void ECS::Collider2D::SetCircleOffset(unsigned int id, float radius)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Collider2DData>(id);
	if (data != nullptr)
	{
		data->CircleRadius = radius;
		data->type = Collider2DType::Circle;
		data->IsChange = true;
	}
	else
	{
		PixelLog::Error("[Collider2D][SetCircleOffset] Not Find Component");
	}
}

void ECS::Collider2D::SetCenter(unsigned int id, float x, float y)
{
	auto registry = GetRegistry();
	auto data = registry->Get<Collider2DData>(id);
	if (data != nullptr)
	{
		data->Center = { x,y };
		data->IsChange = true;
	}
	else
	{
		PixelLog::Error("[Collider2D][SetCenter] Not Find Component");
	}
}
