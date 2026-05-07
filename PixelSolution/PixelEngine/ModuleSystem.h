#pragma once
class ModuleSystem
{
public:
	ModuleSystem();
	virtual ~ModuleSystem();

	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Clear() = 0;
	virtual void LuaBind() = 0;
};

