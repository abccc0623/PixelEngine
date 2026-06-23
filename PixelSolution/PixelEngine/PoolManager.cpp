#include "pch.h"
#include "PoolManager.h"
#include "PixelEngineAPI.h"
#include "EntityObject.h"
void ECS::PoolManager::Initialize()
{

}

void ECS::PoolManager::Release()
{
	PoolList.clear();
	PoolCreateFunctionList.clear();
	PoolActiveFunctionList.clear();
}

unsigned int ECS::PoolManager::Active(const std::string& poolName)
{
	auto find = PoolList.find(poolName);
	if (find == PoolList.end())
	{
		PoolList.insert({ poolName ,std::vector<unsigned int>() });
	}
	auto& pool = PoolList[poolName];
	for (int i = 0; i < pool.size(); i++)
	{
		auto ID = pool[i];
		auto entity = FindEntity(ID);
		if (entity->GetActive() == false)
		{
			entity->SetActive(true);
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
			PoolList[poolName].push_back(id);
			return id;
		}
	}

	PixelLog::Error("[Pool] Not Find : AutoCreateFunction");
	return 0;
}

void ECS::PoolManager::Disable(const std::string& poolName, unsigned int id)
{
	auto find = PoolList.find(poolName);
	if (find == PoolList.end())
	{
		PoolList.insert({ poolName ,std::vector<unsigned int>() });
	}
	auto& pool = PoolList[poolName];
	if (std::find(pool.begin(), pool.end(), id) == pool.end())
	{
		auto entity = FindEntity(id);
		entity->SetActive(false);
		pool.push_back(id);
	}
	else
	{
		auto entity = FindEntity(id);
		entity->SetActive(false);
	}
}

void ECS::PoolManager::Clear(const std::string& poolName)
{
	auto find = PoolList.find(poolName);
	if (find != PoolList.end())
	{
		PoolList[poolName].clear();
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
