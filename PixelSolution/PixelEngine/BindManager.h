#pragma once
#include "EngineManager.h"
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <string>
#include "GlobalBind.h"

class GameObject;
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
	void BindTransform();
	void BindRenderer2D();
	void BindDebugCamera();
	void BindCamera();
	void BindGameObject();
};