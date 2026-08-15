#pragma once
#include "ISystem.h"

namespace ECS
{
	class UISilderSystem : public ISystem
	{
	public:
		void Update(Registry* registry) override;
		void EditorUpdate(Registry* registry) override;
		void Release() override;
	};
}
