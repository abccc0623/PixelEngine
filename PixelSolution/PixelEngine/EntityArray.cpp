#include "pch.h"
#include "EntityArray.h"
#include "Entity.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
ECS::EntityArray::EntityArray()
{
	entityArray.reserve(1000);
	entityIDArray.reserve(1000);
}

ECS::EntityArray::~EntityArray()
{

}

void ECS::EntityArray::Clear()
{
	entityArray.clear();
}

uint32_t ECS::EntityArray::Create(const std::string& scriptName)
{
	int index = entityArray.size();

	//배열의 빈곳이 없는 상태라면
	if (freeIndices.size() == 0)
	{
		entityIDArray.emplace_back(index, 1);
		entityArray.emplace_back(entityIDArray[index].value, scriptName);
	}
	else
	{
		index = freeIndices.front();
		freeIndices.pop();
		entityArray[index] = ECS::Entity(entityIDArray[index].value, scriptName);
	}
	entityArray[index].SpawnCall();
	return entityIDArray[index].value;
}


void ECS::EntityArray::Destroy(ECS::EntityID id)
{
	uint32_t index = id.Index();
	//배열이 초과했을 경우
	if (index >= entityIDArray.size()) return;

	//버전이 다를경우
	if (entityIDArray[index].Version() != id.Version()) return;

	//버전업 시킨다
	uint32_t nextVersion = id.Version() + 1;
	entityIDArray[index].SetValue(index, nextVersion);

	entityArray[index].Clear();
	freeIndices.push(index);
}
