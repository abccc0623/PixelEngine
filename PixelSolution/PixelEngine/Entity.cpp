#include "pch.h"
#include "Entity.h"
#include "PixelEngineAPI.h"
#include "EntityObject.h"
sol::object ECS::Entity::GetValue(unsigned int id, const char* memberName)
{
	auto target = FindEntity(id);
	if (target == nullptr)
	{
		return sol::nil;
	}
	return target->GetValue(memberName);
}

void ECS::Entity::SetValue(unsigned int id, const char* memberName, sol::object value)
{
	auto target = FindEntity(id);
	if (target == nullptr)
	{
		PixelLog::Error("[Entity][SetValue] Not Find ID");
		return;
	}
	return target->SetValue(memberName, value);
}
