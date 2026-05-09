#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include "IComponentArray.h"
#include "ComponentArray.h"
namespace ECS
{
	class Registry
	{
	public:
		Registry();
		~Registry();

		template<typename T>
		void AddComponent(unsigned int entityID)
		{
			std::type_index key = typeid(T);
			auto find = componentArrays.find(key);
			if (find != componentArrays.end())
			{
				componentArrays[key] = new ComponentArray<T>();
			}
			else
			{
				componentArrays.insert({key,new ComponentArray<T>()});
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
				void* rawData = componentArrays[key]->Get(entityID);
				return static_cast<T*>(rawData);
			}
			return nullptr;
		}

	private:
		std::unordered_map<std::type_index,IComponentArray*> componentArrays;
	};
}

