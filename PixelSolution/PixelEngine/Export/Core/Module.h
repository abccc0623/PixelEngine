#pragma once
#include "Core/PixelObject.h"
#include <string>
#include <vector>
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

protected:
	//Transform* transform;
	//GameObject* targetObject;
private:
};

