#pragma once
#include "ISystem.h"

namespace ECS
{
	class Renderer2DSystem :public ISystem
	{
	public:
		Renderer2DSystem();
		virtual ~Renderer2DSystem();

		void Update(Registry* registry) override;
		void Release() override;
	};
};

