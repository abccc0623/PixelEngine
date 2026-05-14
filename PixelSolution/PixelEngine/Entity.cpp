#include "pch.h"
#include "Entity.h"
#include "EntityLuaData.h"
#include <sol/sol.hpp>
ECS::Entity::Entity(unsigned int ID, const std::string& scriptName)
{
	this->ID = ID;
	memset(luaDataBuffer, 0, sizeof(luaDataBuffer));
	new(luaDataBuffer) EntityLuaData(this, scriptName);
	bHasLuaData = true; // 조립 완료!
	Active = true;
}
ECS::Entity::Entity(Entity&& other) noexcept : bHasLuaData(other.bHasLuaData)
{
	if (other.bHasLuaData)
	{
		new(this->luaDataBuffer) EntityLuaData(std::move(*other.GetLua()));
		other.bHasLuaData = false;
	}
}
ECS::Entity& ECS::Entity::operator=(Entity&& other) noexcept
{
	if (this == &other) return *this;

	if (this->bHasLuaData)
	{
		GetLua()->~EntityLuaData();
	}
	this->bHasLuaData = other.bHasLuaData;
	if (other.bHasLuaData)
	{
		new(this->luaDataBuffer) EntityLuaData(std::move(*other.GetLua()));
		other.bHasLuaData = false;
	}

	return *this;
}
ECS::Entity::~Entity()
{
	if (bHasLuaData)
	{
		GetLua()->~EntityLuaData();
		bHasLuaData = false;
	}
}
void ECS::Entity::Clear()
{
	if (bHasLuaData)
	{
		GetLua()->~EntityLuaData();
		bHasLuaData = false;
		Active = false;
	}
}
void ECS::Entity::SpawnCall()
{
	if (Active == true)
	{
		GetLua()->FunctionCall("Spawn");
	}
}

void ECS::Entity::UpdateCall(float DTime)
{
	if (Active == true)
	{
		//GetLua()->FunctionCall("Update", DTime);
	}
}

unsigned int ECS::Entity::GetID()
{
	return ID;
}

ECS::EntityLuaData* ECS::Entity::GetLua()
{
	return reinterpret_cast<EntityLuaData*>(luaDataBuffer);
}

