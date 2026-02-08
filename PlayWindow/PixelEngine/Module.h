#pragma once
#include "BaseModule.h"
#include <string>
#include <sol/forward.hpp>
class GameObject;
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
	static sol::state* GetLua();
protected:
	GameObject* targetObject;
	std::string className;
	friend GameObject;
};

