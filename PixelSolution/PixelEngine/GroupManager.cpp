#include "pch.h"
#include "GroupManager.h"
#include <algorithm>

void ECS::GroupManager::Initialize()
{
}

void ECS::GroupManager::Release()
{
	GroupList.clear();
}



void ECS::GroupManager::Set(const std::string& groupName, unsigned int id)
{
	auto find = GroupList.find(groupName);
	if (find == GroupList.end())
	{
		GroupList.insert({ groupName, std::vector<unsigned int>() });
	}

	auto& group = GroupList[groupName];
	if (std::find(group.begin(), group.end(), id) == group.end())
	{
		group.push_back(id);
	}
}

void ECS::GroupManager::Remove(const std::string& groupName, unsigned int id)
{
	auto find = GroupList.find(groupName);
	if (find == GroupList.end())
	{
		return;
	}

	auto& group = find->second;
	group.erase(std::remove(group.begin(), group.end(), id), group.end());
	if (group.empty())
	{
		GroupList.erase(find);
	}
}

void ECS::GroupManager::RemoveFromAll(unsigned int id)
{
	for (auto it = GroupList.begin(); it != GroupList.end();)
	{
		auto& group = it->second;
		group.erase(std::remove(group.begin(), group.end(), id), group.end());
		if (group.empty())
		{
			it = GroupList.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void ECS::GroupManager::Clear(const std::string& groupName)
{
	GroupList.erase(groupName);
}

const std::vector<unsigned int>* ECS::GroupManager::Get(const std::string& groupName) const
{
	auto find = GroupList.find(groupName);
	if (find == GroupList.end())
	{
		return nullptr;
	}
	return &find->second;
}
