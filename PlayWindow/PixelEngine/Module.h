#pragma once
#include "PixelObject.h"
#include "BaseModule.h"
#include <string>
#include <sol/forward.hpp>
#include <vector>
class GameObject;
class LuaManager;
class Module : public BaseModule
{
public:
	Module();
	virtual ~Module();

	virtual void Awake();
	virtual void Start();
	virtual void Update();
	virtual void MatrixUpdate();
	virtual void PhysicsUpdate();
	virtual void LastUpdate();

	std::string GetClassNameString();
	
protected:
	PPointer<GameObject> targetObject;
	std::string className;
	friend GameObject;
	static sol::state* GetLuaState();
	static void AddLuaAPI(std::string className, std::vector<std::string> functionName);
private:
	static LuaManager* lua;
};

