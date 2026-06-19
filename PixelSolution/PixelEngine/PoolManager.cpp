#include "pch.h"
#include "PoolManager.h"

void ECS::PoolManager::Initialize()
{

}

void ECS::PoolManager::Release()
{
	PoolList.clear();
}

unsigned int ECS::PoolManager::CreateEntity(const std::string& poolName, const std::string& scriptName)
{
	return 0;
}
