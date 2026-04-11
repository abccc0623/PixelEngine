#pragma once
#include "Core/Module.h"
#include "SPointer.h"
#include "WPointer.h"
#include "BitMask8.h"
template <typename T> class WPointer;
class GameObject;
class Movement :public Module
{
public:
	Movement();
	~Movement();
	void Update() override;

	float stopDistance = 0;
	float speed = 1;
	void MoveToTarget(GameObject* target);
	void StopMove();
	void AddCompleteCallBack(std::string functionName);
	void AddStartedCallBack(std::string functionName);
private:
	WPointer<GameObject> target;
	sol::function complete;
	sol::function started;
	sol::table instance;

	BitMask8 bitmask;
	void CallFunction(sol::function& call);

	enum MOVE_STATE
	{
		NONE	= 0,
		MOVE	= 1 << 0,
		STOP	= 1 << 1,
		START	= 1 << 2,
		END		= 1 << 3,
	};
};

