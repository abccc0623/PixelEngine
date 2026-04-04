#pragma once
#include "Core/Module.h"
#include "SPointer.h"
#include "WPointer.h"
template <typename T> class WPointer;
class GameObject;
class Movement :public Module
{
public:
	Movement();
	~Movement();
	
	void Update() override;

	float StartDistance = 0;
	float StopDistance = 0;
	float speed = 1;
	void MoveToTarget(GameObject* target);
private:
	WPointer<GameObject> target;
};

