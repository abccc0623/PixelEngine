#pragma once
#include "Core/PixelObject.h"
#include "Type/GlobalEnum.h"
#include <string>
#include <vector>
class GameObject;
class Transform;
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

	PIXEL_ENGINEDLL MODULE_TYPE GetType();
	PIXEL_ENGINEDLL GameObject* GetGameObject();
protected:
	Transform* transform;
	GameObject* targetObject;
	MODULE_TYPE type;
private:
	friend GameObject;
};

