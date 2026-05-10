#pragma once
#include "IComponentArray.h"
#include <vector>
namespace ECS
{
	template <typename T>
	class ComponentArray : public IComponentArray
	{
	public:
		ComponentArray() {}
		virtual ~ComponentArray() {};
		void Destroy(unsigned int entityID) override
		{

		}
		void Create(unsigned int entityID) override
		{
			if (entityToIndexMap.find(entityID) != entityToIndexMap.end())
			{
				return;
			}

			size_t newIndex = componentArray.size();
			entityToIndexMap[entityID] = newIndex;
			indexToEntityMap[newIndex] = entityID;

			componentArray.push_back(T());
		}

		void* Get(unsigned int entityID)
		{
			if (entityToIndexMap.find(entityID) != entityToIndexMap.end())
			{
				size_t realIndex = entityToIndexMap[entityID];
				return static_cast<void*>(&componentArray[realIndex]);
			}
			else
			{
				return nullptr;
			}
		}
		unsigned int GetEntityID(int index) override
		{
			return indexToEntityMap[index];
		}

		std::vector<T>& GetArray()
		{
			return componentArray;
		}

		void Update()
		{

		}
	private:
		std::vector<T> componentArray;

		std::unordered_map<unsigned int, size_t> entityToIndexMap;
		std::unordered_map<size_t, unsigned int> indexToEntityMap;
	};
}

