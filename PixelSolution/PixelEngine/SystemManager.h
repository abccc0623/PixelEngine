#pragma once
#include "EngineManager.h"
#include <vector>
class ModuleSystem;
class TimeManager;
class SystemManager : public EngineManager
{
public:
	SystemManager();
	virtual ~SystemManager();

	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;
private:
	TimeManager* timeManager;
private:
	std::vector<ModuleSystem*> SystemList;
};

