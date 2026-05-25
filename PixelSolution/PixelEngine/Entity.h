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
		void OnCollisionEnter(unsigned int TargetID);
		void OnCollisionExit(unsigned int TargetID);
		void OnEvent(std::string eventName, sol::table event);
		unsigned int GetID();
	public:
		bool Active;
	private:
		std::string scriptName;
		sol::table instance;

		sol::protected_function OnCollisionEnterFunc;
		sol::protected_function OnCollisionExitFunc;
		unsigned int ID;
	private:
		static LuaManager* lua;
	};
};

