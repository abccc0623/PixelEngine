#pragma once

#include <vector>
#include <unordered_map>
#include <string>
namespace ECS
{
	class GroupManager
	{
	public:
		void Initialize();
		void Release();

		void Set(const std::string& groupName, unsigned int id);
		void Remove(const std::string& groupName, unsigned int id);
		void RemoveFromAll(unsigned int id);
		void Clear(const std::string& groupName);
		const std::vector<unsigned int>* Get(const std::string& groupName) const;
	private:
		std::unordered_map<std::string, std::vector<unsigned int>> GroupList;
	};
};

