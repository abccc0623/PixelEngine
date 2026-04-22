#pragma once
#include "Core/Module.h"
#include "SPointer.h"
#include "WPointer.h"
#include "BitMask8.h"
#include "Type/PVector3.h"
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
	void MoveToPosition(float x, float y, float z);
	void MoveToTarget(GameObject* target);
	void StopMove();
	void AddCompleteCallBack(std::string functionName);
	void AddStartedCallBack(std::string functionName);
	void AddDirectionCallBack(std::string functionName);
private:
	bool isTargetingPosition = false; // 좌표로 이동 중인지 체크하는 플래그
	PVector3 targetPosition;
	WPointer<GameObject> target;
	sol::function complete;
	sol::function started;
	sol::function directionChange;
	sol::table instance;


	PVector3 lastDirection;
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

