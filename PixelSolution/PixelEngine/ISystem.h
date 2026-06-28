#pragma once
namespace ECS
{
	class Registry;
	class ISystem
	{
	public:
		ISystem();
		virtual ~ISystem();

		virtual void Update(Registry* registry) = 0;
		virtual void Release() = 0;
		virtual void EditorUpdate(Registry* registry) = 0;
	private:
	};
};

