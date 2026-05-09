#pragma once
#include <vector>
#include <queue>
#include "Entity.h"
#include "EntityID.h"
namespace ECS 
{
	class EntityArray
	{
	public:
		EntityArray();
		~EntityArray();

		void Clear();
		uint32_t Create(const std::string& scriptName);
		void Update();
		void Destroy(ECS::EntityID id);
	private:
		std::vector<ECS::Entity>	entityArray;
		std::vector<ECS::EntityID>	entityIDArray;
		std::queue<int>	freeIndices;
	};
};

