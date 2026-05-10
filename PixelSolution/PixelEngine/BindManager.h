#pragma once
#include "EngineManager.h"
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <string>

class Module;
class BindManager : public EngineManager
{
public:
	BindManager();
	~BindManager();

	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

private:
	void BindLuaScript();
	void BindMovement();
	void BindPVector3();
	void BindEnum();
	void BindPhysics2D();
	void BindEntity();
private:

};