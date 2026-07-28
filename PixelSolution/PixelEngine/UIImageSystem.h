#pragma once
#include "ISystem.h"
namespace ECS
{
	class UIImageSystem : public ISystem
	{
	public:
		UIImageSystem();
		virtual ~UIImageSystem();

		void Update(ECS::Registry* registry) override;
		void EditorUpdate(Registry* registry) override;
		void Release() override;
	};
};


