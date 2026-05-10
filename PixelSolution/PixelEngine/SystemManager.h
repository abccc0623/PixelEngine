#pragma once
#include <vector>

class TimeManager;
namespace ECS 
{
	class ISystem;
	class Registry;
	class SystemManager
	{
	public:
		SystemManager();
		virtual ~SystemManager();

		void Initialize();
		void Update(ECS::Registry* registry);
		void Release();
	private:
		std::vector<ISystem*> SystemList;
	};
}


