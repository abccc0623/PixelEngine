#pragma once
#include "ISystem.h"
namespace ECS
{
	class GraphicsSystem :public ISystem
	{
	public:
		GraphicsSystem();
		virtual ~GraphicsSystem();

		void Update(ECS::Registry* registry) override;
		void EditorUpdate(Registry* registry) override;
		void Release() override;
	private:
	};
}




