#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include "IComponentArray.h"
#include "ComponentArray.h"
#include "ChunkedArray.h"
namespace ECS
{
	class Registry
	{
	public:
		Registry();
		~Registry();

		void Remove(unsigned int entityID);

		template<typename T>
		void AddComponent(unsigned int entityID)
		{
			std::type_index key = typeid(T);
			auto find = componentArrays.find(key);
			if (find == componentArrays.end())
			{
				componentArrays.insert({ key,new ComponentArray<T>() });
			}
			componentArrays[key]->Create(entityID);

		};
		template<typename T>
		T* Get(unsigned int entityID)
		{
			std::type_index key = typeid(T);
			auto find = componentArrays.find(key);
			if (find != componentArrays.end())
			{
				void* rawData = find->second->Get(entityID);
				return static_cast<T*>(rawData);
			}
			return nullptr;
		}
		template<typename T>
		bool Has(unsigned int entityID)
		{
			std::type_index key = typeid(T);
			auto find = componentArrays.find(key);
			if (find == componentArrays.end())
			{
				return false;
			}
			return true;
		}


		template<typename T>
		unsigned int GetEntityID(int index)
		{
			std::type_index key = typeid(T);
			auto find = componentArrays.find(key);
			if (find != componentArrays.end())
			{
				return find->second->GetEntityID(index);
			}
			return -1;
		}

		template<typename T>
		std::vector<T>& GetArray()
		{
			std::type_index key = std::type_index(typeid(T));
			auto find = componentArrays.find(key);

			if (find == componentArrays.end())
			{
				componentArrays[key] = new ComponentArray<T>();
			}
			auto* realChild = static_cast<ComponentArray<T>*>(componentArrays[key]);
			return realChild->GetArray();
		}


		template<typename T>
		ChunkedArray<T, 128>& GetChunkedArray()
		{
			std::type_index key = std::type_index(typeid(T));
			auto find = componentArrays.find(key);

			if (find == componentArrays.end())
			{
				componentArrays[key] = new ComponentArray<T>();
			}

			auto* realChild = static_cast<ComponentArray<T>*>(componentArrays[key]);
			return realChild->GetChunk();
		}
	private:
		std::unordered_map<std::type_index, IComponentArray*> componentArrays;
		const int maxsize = 128;
	};

}

