#pragma once
#include "ISystem.h"

namespace ECS
{
	class UITextSystem : public ISystem
	{
	public:
		UITextSystem();
		virtual ~UITextSystem();

		void Update(Registry* registry) override;
		void EditorUpdate(Registry* registry) override;
		void Release() override;
	};
}
