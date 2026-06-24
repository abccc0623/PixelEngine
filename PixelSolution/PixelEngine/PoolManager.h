#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <sol/forward.hpp>
namespace ECS
{
	class PoolManager
	{
	public:
		void Initialize();
		void Release();

		unsigned int Active(const std::string& poolName);
		void Disable(const std::string& poolName, unsigned int id);
		void Clear(const std::string& poolName);
		void SetAutoCreateFunction(const std::string& poolName, sol::function func);
		void SetAutoActiveFunction(const std::string& poolName, sol::function func);

		sol::as_table_t<std::vector<unsigned int>> GetActiveArray(const std::string& poolName);
	private:
		std::unordered_map<std::string, std::vector<unsigned int>> PoolList;
		std::unordered_map<std::string, std::vector<unsigned int>> ActivePoolList;
		std::unordered_map<std::string, std::vector<unsigned int>> InactivePoolList;
		std::unordered_map<std::string, sol::function> PoolCreateFunctionList;
		std::unordered_map<std::string, sol::function> PoolActiveFunctionList;
	};
};

