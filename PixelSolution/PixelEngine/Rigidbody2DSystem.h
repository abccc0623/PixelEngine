#pragma once
#include "ISystem.h"
namespace ECS
{
	class Rigidbody2DSystem :public ISystem
	{
	public:
		Rigidbody2DSystem();
		virtual ~Rigidbody2DSystem();

		void Update(ECS::Registry* registry) override;
		void Release() override;
	};
}


