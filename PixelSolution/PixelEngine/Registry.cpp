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
