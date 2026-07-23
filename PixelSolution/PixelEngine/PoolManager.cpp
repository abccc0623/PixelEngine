#include "pch.h"
#include "PoolManager.h"
#include "PixelEngineAPI.h"
#include "EntityObject.h"
#include "Pool.h"
#include "Entity.h"

void ECS::PoolManager::Initialize()
{

}

void ECS::PoolManager::Release()
{
	poolList.clear();
}

unsigned int ECS::PoolManager::Active(const std::string& scriptName)
{
	auto k = poolList.find(scriptName);
	if (k == poolList.end())
	{
		poolList.insert({ scriptName ,PoolData() });
	}

	//데이터가 없다면
	auto& data = poolList[scriptName];
	if (data.InactiveList.empty())
	{
		for (int i = 0; i < data.CreateCount; i++)
		{
			auto id = Entity_Create(scriptName.c_str());
			Register(scriptName, id);
			Disable(scriptName, id);
		}
	}

	//사용하지 않는 데이터에서 하나꺼내옴
	auto id = data.InactiveList.front();
	data.InactiveList.pop_front();
	if (std::find(data.ActiveList.begin(), data.ActiveList.end(), id) == data.ActiveList.end())
	{
		data.ActiveList.push_back(id);
	}
	Entity_SetActive(id, true);
	return id;
}

void ECS::PoolManager::Register(const std::string& scriptName, unsigned int id)
{
	auto& data = poolList[scriptName];
	const bool isActive = std::find(data.ActiveList.begin(), data.ActiveList.end(), id) != data.ActiveList.end();
	const bool isInactive = std::find(data.InactiveList.begin(), data.InactiveList.end(), id) != data.InactiveList.end();
	if (!isActive && !isInactive)
	{
		data.ActiveList.push_back(id);
	}
}

void ECS::PoolManager::Disable(const std::string& scriptName, unsigned int id)
{
	auto poolIt = poolList.find(scriptName);
	if (poolIt == poolList.end())
	{
		PixelLog::Error("[Pool][Disable] Not Find Pool: " + scriptName);
		return;
	}

	auto& data = poolIt->second;
	auto activeIt = std::find(data.ActiveList.begin(), data.ActiveList.end(), id);
	if (activeIt == data.ActiveList.end())
	{
		auto inactiveIt = std::find(data.InactiveList.begin(), data.InactiveList.end(), id);
		if (inactiveIt != data.InactiveList.end())
		{
			PixelLog::Warn(
				"[Pool][Disable] Entity is already inactive. Pool: " +
				scriptName + ", ID: " + std::to_string(id));
			return;
		}

		PixelLog::Error(
			"[Pool][Disable] Entity does not belong to pool. Pool: " +
			scriptName + ", ID: " + std::to_string(id));
		return;
	}

	data.ActiveList.erase(activeIt);
	data.InactiveList.push_back(id);
	Entity_SetActive(id, false);
}

void ECS::PoolManager::Clear(const std::string& scriptName)
{
	poolList[scriptName].ActiveList.clear();
	poolList[scriptName].InactiveList.clear();
}

int ECS::PoolManager::ActiveCount(const std::string& scriptName)
{
	return (int)poolList[scriptName].ActiveList.size();
}

unsigned int ECS::PoolManager::ActiveID(const std::string& scriptName, int index)
{
	int size = (int)poolList[scriptName].ActiveList.size();
	if (index < size)
	{
		return poolList[scriptName].ActiveList[index];
	}
	return 0;
}

void ECS::PoolManager::SetExpansionSize(const std::string& scriptName, int createCount)
{
	poolList[scriptName].CreateCount = createCount;
}


sol::as_table_t<std::vector<unsigned int>> ECS::PoolManager::GetActiveArray(const std::string& scriptName)
{
	auto k = poolList.find(scriptName);
	if (k == poolList.end())
	{
		poolList.insert({ scriptName ,PoolData() });
	}
	auto& data = poolList[scriptName];
	return sol::as_table(data.ActiveList);
}
