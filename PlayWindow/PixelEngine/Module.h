#pragma once
#include "PixelObject.h"
#include <string>
#include <sol/forward.hpp>
#include <vector>
class GameObject;
class LuaManager;
class Module : public PixelObject
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
	virtual void OnCollisionEnter2D(WPointer<GameObject> target);
	virtual void OnCollision2D(WPointer<GameObject> target);
	virtual void OnCollisionExit2D(WPointer<GameObject> target);
	
	std::string GetClassNameString();
	GameObject* targetObject;
	bool isCollisionModule = false;
protected:
	std::string className;
	friend GameObject;
	static sol::state* GetLuaState();
	static void AddLuaAPI(std::string className, std::vector<std::string> functionName);
private:
	static LuaManager* lua;
};

