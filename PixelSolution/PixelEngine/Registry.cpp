#include "pch.h"
#include "Registry.h"

ECS::Registry::Registry()
{

}

ECS::Registry::~Registry()
{
	for (auto& K : componentArrays)
	{
		delete K.second;
	}
}

void ECS::Registry::Remove(unsigned int entityID)
{
	for (auto& K : componentArrays)
	{
		K.second->Destroy(entityID);
	}
}
