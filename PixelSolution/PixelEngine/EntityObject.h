#pragma once
#include <string>
#include <sol/forward.hpp>

class LuaManager;
namespace ECS
{
	class EntityObject
	{
	public:
		EntityObject();
		~EntityObject();
	public:
		void Create(const std::string& scriptName, unsigned int ID);
		void OnCollisionEnter(unsigned int TargetID);
		void OnCollisionExit(unsigned int TargetID);
		void OnEvent(std::string functionName, sol::object event);
		unsigned int GetID();
		bool GetActive();
		void SetActive(bool isActive);

		void SetLayer(const char* layer);
		const char* GetLayer();

		sol::object GetValue(const char* memberName);
		void SetValue(const char* memberName, sol::object);
	private:
		bool Active;
		std::string layerName = "Default";
		std::string scriptName;
		sol::table instance;
		sol::protected_function OnCollisionEnterFunc;
		sol::protected_function OnCollisionExitFunc;
		unsigned int ID;
	private:
		static LuaManager* lua;
	};
};

