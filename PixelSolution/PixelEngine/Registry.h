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
		void Add(unsigned int entityID)
		{
			std::type_index key = typeid(T);
			auto find = componentArrays.find(key);
			if (find == componentArrays.end())
			{
				componentArrays.insert({ key,new ComponentArray<T>() });
			}
			componentArrays[key]->Create(entityID);
		};
		// Component APIs often need a primary and a companion component. On capacity
		// failure, roll back only this call's additions, preserving existing data.
		template<typename... T>
		bool TryAddComponents(unsigned int entityID)
		{
			if (entityID == ChunkedID::InvalidValue) return false;
			const std::array<bool, sizeof...(T)> existed{ Has<T>(entityID)... };
			(Add<T>(entityID), ...);
			if ((Has<T>(entityID) && ...)) return true;
			size_t index = 0;
			([&]
			{
				if (!existed[index++])
				{
					auto found = componentArrays.find(std::type_index(typeid(T)));
					if (found != componentArrays.end()) found->second->Destroy(entityID);
				}
			}(), ...);
			return false;
		}

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
			return find->second->Has(entityID);
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


		void SetActive(unsigned int entityID, bool active)
		{
			for (auto& k : componentArrays)
			{
				k.second->SetActive(entityID, active);
			}
		}

	private:
		std::unordered_map<std::type_index, IComponentArray*> componentArrays;
		const int maxsize = 128;
	};

}

