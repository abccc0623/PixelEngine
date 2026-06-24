#include "pch.h"
#include "PoolManager.h"
#include "PixelEngineAPI.h"
#include "EntityObject.h"

namespace
{
	void AddUnique(std::vector<unsigned int>& list, unsigned int id)
	{
		if (std::find(list.begin(), list.end(), id) == list.end())
		{
			list.push_back(id);
		}
	}

	void RemoveID(std::vector<unsigned int>& list, unsigned int id)
	{
		auto it = std::remove(list.begin(), list.end(), id);
		if (it != list.end())
		{
			list.erase(it, list.end());
		}
	}
}

void ECS::PoolManager::Initialize()
{

}

void ECS::PoolManager::Release()
{
	PoolList.clear();
	ActivePoolList.clear();
	InactivePoolList.clear();
	PoolCreateFunctionList.clear();
	PoolActiveFunctionList.clear();
}

unsigned int ECS::PoolManager::Active(const std::string& poolName)
{
	auto& pool = PoolList[poolName];
	auto& activePool = ActivePoolList[poolName];
	auto& inactivePool = InactivePoolList[poolName];

	for (int i = 0; i < inactivePool.size(); i++)
	{
		auto ID = inactivePool[i];
		auto entity = FindEntity(ID);
		if (entity != nullptr && entity->GetActive() == false)
		{
			entity->SetActive(true);
			RemoveID(inactivePool, ID);
			AddUnique(activePool, ID);

			auto ActiveFunction = PoolActiveFunctionList.find(poolName);
			if (ActiveFunction != PoolActiveFunctionList.end() && ActiveFunction->second.valid())
			{
				ActiveFunction->second(ID);
			}

			return ID;
		}
	}

	auto CreateFunction = PoolCreateFunctionList.find(poolName);
	if (CreateFunction != PoolCreateFunctionList.end())
	{
		if (CreateFunction->second.valid())
		{
			auto returnData = CreateFunction->second();
			auto id = returnData.get<unsigned int>();
			auto entity = FindEntity(id);
			if (entity != nullptr)
			{
				entity->SetActive(true);
			}
			AddUnique(pool, id);
			AddUnique(activePool, id);
			RemoveID(inactivePool, id);

			auto ActiveFunction = PoolActiveFunctionList.find(poolName);
			if (ActiveFunction != PoolActiveFunctionList.end() && ActiveFunction->second.valid())
			{
				ActiveFunction->second(id);
			}

			return id;
		}
	}

	PixelLog::Error("[Pool] Not Find : AutoCreateFunction");
	return 0;
}

void ECS::PoolManager::Disable(const std::string& poolName, unsigned int id)
{
	auto& pool = PoolList[poolName];
	auto& activePool = ActivePoolList[poolName];
	auto& inactivePool = InactivePoolList[poolName];
	auto entity = FindEntity(id);
	if (entity != nullptr)
	{
		entity->SetActive(false);
	}
	AddUnique(pool, id);
	RemoveID(activePool, id);
	AddUnique(inactivePool, id);
}

void ECS::PoolManager::Clear(const std::string& poolName)
{
	auto find = PoolList.find(poolName);
	if (find != PoolList.end())
	{
		PoolList[poolName].clear();
		ActivePoolList[poolName].clear();
		InactivePoolList[poolName].clear();
	}
	else
	{
		PixelLog::Error("[Pool][Clear][" + poolName + "]" + ": Not Find Name");
	}
}

void ECS::PoolManager::SetAutoCreateFunction(const std::string& poolName, sol::function func)
{
	auto find = PoolCreateFunctionList.find(poolName);
	if (find != PoolCreateFunctionList.end())
	{
		PixelLog::Warn("[Pool][SetAutoCreateFunction]The existing function has been overwritten");
	}
	PoolCreateFunctionList[poolName] = func;
}

void ECS::PoolManager::SetAutoActiveFunction(const std::string& poolName, sol::function func)
{
	auto find = PoolActiveFunctionList.find(poolName);
	if (find != PoolActiveFunctionList.end())
	{
		PixelLog::Warn("[Pool][SetAutoActiveFunction]The existing function has been overwritten");
	}
	PoolActiveFunctionList[poolName] = func;
}

sol::as_table_t<std::vector<unsigned int>> ECS::PoolManager::GetActiveArray(const std::string& poolName)
{
	auto find = ActivePoolList.find(poolName);
	if (find == ActivePoolList.end())
	{
		PixelLog::Warn("[Pool][GetActiveArray] Not Find Name");
		return sol::as_table(std::vector<unsigned int>());
	}

	return sol::as_table(find->second);
}
