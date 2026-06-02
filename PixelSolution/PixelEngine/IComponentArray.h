#pragma once

namespace ECS
{
	class IComponentArray
	{
	public:
		virtual ~IComponentArray() = default;
		virtual void Destroy(unsigned int entityID) = 0;
		virtual void Create(unsigned int entityID) = 0;
		virtual void* Get(unsigned int entityID) = 0;
		virtual bool Has(unsigned int entityID) = 0;
		virtual unsigned int GetEntityID(int index) = 0;
		virtual void SetActive(unsigned int entityID, bool active) = 0;
		virtual void Update() = 0;

		void (*ForEach)(void*) = nullptr;
	};
};

