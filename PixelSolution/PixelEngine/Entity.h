#pragma once
#include <string>
namespace ECS
{
	class EntityLuaData;
	class Entity
	{
	public:
		Entity(unsigned int ID, const std::string& scriptName);
		Entity(Entity&& other) noexcept;
		Entity& operator=(Entity&& other) noexcept;
		Entity(const Entity&) = delete;
		Entity& operator=(const Entity&) = delete;
		~Entity();
	public:
		void Clear();
		void SpawnCall();
		void UpdateCall(float DTime);
		unsigned int GetID();
	public:
		bool Active;
	private:
		alignas(16) unsigned char luaDataBuffer[256];
		bool bHasLuaData = false;
		EntityLuaData* GetLua();
		unsigned int ID;
	};
};

