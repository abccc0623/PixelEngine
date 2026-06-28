#pragma once
#include "ISystem.h"
namespace ECS
{
	class Animation2DSystem : public ISystem
	{
	public:
		Animation2DSystem();
		virtual ~Animation2DSystem();

		void Update(Registry* registry) override;
		void EditorUpdate(Registry* registry) override;
		void Release() override;
	};
}

