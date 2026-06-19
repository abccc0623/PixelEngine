#pragma once
#include <vector>
#include <unordered_map>
#include <string>
namespace ECS
{
	class PoolManager
	{
	public:
		void Initialize();
		void Release();

		unsigned int CreateEntity(const std::string& poolName, const std::string& scriptName);
	private:
		std::unordered_map<std::string, std::vector<unsigned int>> PoolList;
	};
};

