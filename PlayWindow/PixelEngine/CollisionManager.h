#pragma once
#include "EngineManager.h"
class CollisionManager : public EngineManager
{
public:
	CollisionManager();
	~CollisionManager();

	void Initialize() override;
	void Update() override;
	void Release() override;
};

