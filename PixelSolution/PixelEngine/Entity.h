#pragma once
#include <string>
#include <sol/forward.hpp>

class LuaManager;
namespace ECS
{
	class Entity
	{
	public:
		Entity();
		~Entity();
	public:
		void Create(const std::string& scriptName, unsigned int ID);
		void Clear();

		unsigned int GetID();
	public:
		bool Active;
	private:
		std::string scriptName;
		sol::table instance;
		unsigned int ID;
	private:
		static LuaManager* lua;
	};
};

