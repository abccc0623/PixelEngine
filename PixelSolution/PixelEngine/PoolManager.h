#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <sol/forward.hpp>
#include <deque>
namespace ECS
{
	struct PoolData
	{
		std::vector<unsigned int> ActiveList;
		std::deque<unsigned int> InactiveList;
	};

	class PoolManager
	{
	public:
		void Initialize();
		void Release();

		unsigned int Active(const std::string& scriptName);
		void Disable(const std::string& scriptName, unsigned int id);
		void Clear(const std::string& scriptName);
		int ActiveCount(const std::string& scriptName);
		unsigned int ActiveID(const std::string& scriptName, int index);


		sol::as_table_t<std::vector<unsigned int>> GetActiveArray(const std::string& scriptName);
	private:
		std::unordered_map<std::string, PoolData> poolList;
	};
};

