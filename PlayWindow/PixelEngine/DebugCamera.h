#pragma once
#include "Module.h"
class Transform;
struct RenderingData;
class GameObject;
class DebugCamera : public Module
{
public:
	DebugCamera();
	~DebugCamera();

	void Start() override;
	void Update() override;
	void LastUpdate() override;
private:
	SPointer<Transform> transform;
	RenderingData* rendering;
	float moveSpeed;
	float RotationSpeed;
	bool isMove;
	int mLastMousePosX;
	int mLastMousePosY;
};

