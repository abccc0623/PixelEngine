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
	Transform* transform;
	RenderingData* rendering;
	float moveSpeed;
	bool isMove;
	float RotationSpeed;
	int mLastMousePosX;
	int mLastMousePosY;
};

