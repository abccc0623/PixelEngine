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
		for (int i = 0; i < 10; i++)
		{
			auto id = Entity::Create(scriptName.c_str());
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
	Entity::SetActive(id, true);
	return id;
}

void ECS::PoolManager::Disable(const std::string& scriptName, unsigned int id)
{
	//데이터 찾기
	auto k = poolList.find(scriptName);
	if (k == poolList.end())
	{
		poolList.insert({ scriptName ,PoolData() });
	}
	//데이터 삭제
	auto& data = poolList[scriptName];
	for (int i = 0; i < data.ActiveList.size(); i++)
	{
		if (data.ActiveList[i] == id)
		{
			data.ActiveList.erase(data.ActiveList.begin() + i);
			if (std::find(data.InactiveList.begin(), data.InactiveList.end(), id) == data.InactiveList.end())
			{
				data.InactiveList.push_back(id);
			}
			Entity::SetActive(id, false);
			return;
		}
	}
	//만약 여기까지 내려온다면 기존에 Pool에 있던애가 아님
	auto it = std::find(data.InactiveList.begin(), data.InactiveList.end(), id);
	if (it == data.InactiveList.end())
	{
		data.InactiveList.push_back(id);
	}
	Entity::SetActive(id, false);
}

void ECS::PoolManager::Clear(const std::string& scriptName)
{
	poolList[scriptName].ActiveList.clear();
	poolList[scriptName].InactiveList.clear();
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
