#pragma once
#include "IComponentArray.h"
#include "ChunkedArray.h"
#include <limits>
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
			auto id = ECS::ChunkedID(entityToIndexMap[entityID]);
			chunkedArray.Remove(id);
			entityToIndexMap.erase(entityID);
			indexToEntityMap.erase(id.Index());
		}
		void Create(unsigned int entityID) override
		{
			if (entityToIndexMap.find(entityID) != entityToIndexMap.end())
			{
				return;
			}

			ChunkedID id = chunkedArray.Add();
			entityToIndexMap[entityID] = id.value;
			indexToEntityMap[id.Index()] = entityID;
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
		bool Has(unsigned int entityID) override
		{
			return entityToIndexMap.find(entityID) != entityToIndexMap.end();
		}
		unsigned int GetEntityID(int index) override
		{
			// operator[]는 없는 인덱스에 Entity 0을 자동 등록하므로 사용하지 않는다.
			auto found = indexToEntityMap.find(index);
			if (found == indexToEntityMap.end())
			{
				return (std::numeric_limits<unsigned int>::max)();
			}
			return found->second;
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
		void SetActive(unsigned int entityID, bool active) override
		{
			if (entityToIndexMap.find(entityID) != entityToIndexMap.end())
			{
				auto id = ECS::ChunkedID(entityToIndexMap[entityID]);
				chunkedArray.SetLife(id.Index(), active);
			}
		}
	private:
		std::vector<T> componentArray;
		ChunkedArray<T, 128> chunkedArray;
		std::unordered_map<unsigned int, unsigned int> entityToIndexMap;
		std::unordered_map<unsigned int, unsigned int> indexToEntityMap;
	};
}

