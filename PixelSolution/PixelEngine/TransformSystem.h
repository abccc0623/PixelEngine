#pragma once
#include "ModuleSystem.h"
class TransformSystem : public ModuleSystem
{
public:
	void Initialize() override;
	void Update(float deltaTime) override;
	void Clear() override;
	void LuaBind() override;
};

