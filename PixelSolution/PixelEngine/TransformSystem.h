#pragma once
#include "ISystem.h"
namespace ECS
{
	class TransformSystem : public ISystem
	{
	public:
		TransformSystem();
		virtual ~TransformSystem();

		void Update(ECS::Registry* registry) override;
		void EditorUpdate(Registry* registry) override;
		void Release() override;
	};
};

