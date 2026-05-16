#pragma once
#include "IComponentArray.h"
#include "ChunkedArray.h"
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
			// 1. 삭제할 엔티티가 실제로 존재하는지 확인
			if (entityToIndexMap.find(entityID) == entityToIndexMap.end())
			{
				return;
			}
			auto indexOfRemoved = entityToIndexMap[entityID];
			chunkedArray.Remove(ECS::ChunkedID(indexOfRemoved));
			entityToIndexMap.erase(entityID);
			indexToEntityMap.erase(indexOfRemoved);
		}
		void Create(unsigned int entityID) override
		{
			if (entityToIndexMap.find(entityID) != entityToIndexMap.end())
			{
				return;
			}

			ChunkedID id = chunkedArray.Add();
			entityToIndexMap[entityID] = id.value;
			indexToEntityMap[id.value] = entityID;
		}

		void* Get(unsigned int entityID)
		{
			if (entityToIndexMap.find(entityID) != entityToIndexMap.end())
			{
				auto id = ECS::ChunkedID(entityToIndexMap[entityID]);
				auto Data = chunkedArray.Get(id);
				return static_cast<void*>(Data);
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

		ChunkedArray<T, 128>& GetChunk()
		{
			return chunkedArray;
		}
		void Update()
		{

		}
	private:
		std::vector<T> componentArray;
		ChunkedArray<T, 128> chunkedArray;
		std::unordered_map<unsigned int, unsigned int> entityToIndexMap;
		std::unordered_map<unsigned int, unsigned int> indexToEntityMap;
	};
}

