#pragma once
#include <string>
#include <sol/forward.hpp>
class LuaManager;
namespace ECS
{
	namespace Pixel::Utils
	{
		const uint32_t FNV_OFFSET_BASIS = 2166136261u;
		const uint32_t FNV_PRIME = 16777619u;
		constexpr uint32_t HashString(const char* str, uint32_t hash = FNV_OFFSET_BASIS) 
		{
			return (*str == '\0') ? hash : HashString(str + 1, (hash ^ static_cast<uint32_t>(*str)) * FNV_PRIME);
		}
	}

	class Entity;
	class EntityLuaData
	{
	public:
		EntityLuaData(Entity* target,std::string scriptName);
		~EntityLuaData();
		EntityLuaData(EntityLuaData&& other) noexcept = default;
		void SpawnCall();
		void UpdateCall();

		template <typename... Args>
		void FunctionCall(const char* functionName, Args&&... args);
		
	private:
		std::string scriptName;
		sol::table instance;
		sol::function spawn;
		sol::function update;
		std::unordered_map<uint32_t, sol::function> functionList;
	private:
		static constexpr uint32_t spawnID = Pixel::Utils::HashString("Spawn");
		static constexpr uint32_t updateID = Pixel::Utils::HashString("Update");
		static LuaManager* lua;
	};


	template <typename... Args>
	void EntityLuaData::FunctionCall(const char* functionName, Args&&... args)
	{
		uint32_t hash = Pixel::Utils::HashString(functionName);
		sol::function targetFunction;
		switch (hash)
		{
		case spawnID: targetFunction = spawn; break;
		case updateID: targetFunction = update; break;
		default:
			auto find = functionList.find(hash);
			if (find != functionList.end())
			{
				targetFunction = find->second;
			}
			else
			{
				sol::function func = instance[functionName];
				if (func.valid())
				{
					functionList.insert({ hash ,func });
					targetFunction = func;
				}
			}
			break;
		}
		if (targetFunction.valid())
		{
			auto result = targetFunction(instance, std::forward<Args>(args)...);
			if (!result.valid())
			{
				sol::error err = result;
				std::string what = err.what();
				std::string name(functionName);
				PixelLog::Error("--- LUA ERROR ---");
				PixelLog::Error("functionName :" + name);
				PixelLog::Error(what);
				PixelLog::Error("-----------------------");
			}
		}
	}
};

