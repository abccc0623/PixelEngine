#pragma once
#include "ISystem.h"
namespace ECS
{
	class CameraSystem : public ISystem
	{
	public:
		CameraSystem();
		virtual ~CameraSystem();

		void Update(Registry* registry) override;
		void Release() override;
	};
};

