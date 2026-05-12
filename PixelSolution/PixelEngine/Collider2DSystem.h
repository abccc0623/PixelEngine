#pragma once
#include "ISystem.h"

class PhysManager;
namespace ECS
{
	class Collider2DSystem :public ISystem
	{
	public:
		Collider2DSystem();
		virtual ~Collider2DSystem();

		void Update(Registry* registry) override;
		void Release() override;
	private:
		PhysManager* phys;
	};
};

