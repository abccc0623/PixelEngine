#include "pch.h"
#include "DebugCamera.h"
#include "PixelGraphicsAPI.h"
#include "PixelEngineAPI.h"
#include "GameObject.h"
#include "Transform.h"

DebugCamera::DebugCamera():
	rendering(nullptr),
	transform(nullptr),
	isMove(false),
	moveSpeed(10),
	RotationSpeed(10),
	mLastMousePosX(0),
	mLastMousePosY(0)
{

}

DebugCamera::~DebugCamera()
{
	transform = nullptr;
	rendering = nullptr;
}

void DebugCamera::Start()
{
	transform = targetObject->GetModule<Transform>();
	rendering = GetRenderingData();
	rendering->Type = CAMERA;
	transform->AddPosition(transform->GetLookVector() * -3);
}

void DebugCamera::Update()
{
	auto DTime = GetDeltaTime();
	if (GetKey('W') == true) { transform->AddPosition(transform->GetLookVector() * moveSpeed * DTime); isMove = true; }
	if (GetKey('S') == true) { transform->AddPosition(transform->GetLookVector() * -moveSpeed * DTime); isMove = true; }
	if (GetKey('D') == true) { transform->AddPosition(transform->GetRightVector() * moveSpeed * DTime); isMove = true; }
	if (GetKey('A') == true) { transform->AddPosition(transform->GetRightVector() * -moveSpeed * DTime); isMove = true; }


	if (GetKeyDown(VK_RBUTTON) == true)
	{
		ShowCursor(false);
		mLastMousePosX = GetMousePosition_X();
		mLastMousePosY = GetMousePosition_Y();
	}
	if (GetKeyUp(VK_RBUTTON) == true)
	{
		ShowCursor(true);
		mLastMousePosX = GetMousePosition_X();
		mLastMousePosY = GetMousePosition_Y();
	}
	if (GetKey(VK_RBUTTON) == true)
	{
		int x = GetMousePosition_X();
		int y = GetMousePosition_Y();
		float dx = (x - mLastMousePosX) * 3.141592f / 180.0f;
		float dy = (y - mLastMousePosY) * 3.141592f / 180.0f;
		Vector3 Rotation = { -dy * RotationSpeed, -dx * RotationSpeed, 0 };
		transform->AddRotation(Rotation);
		SetCursorPos((int)mLastMousePosX, (int)mLastMousePosY);

		Vector3 Pos = transform->GetPosition();
		Vector3 Rot = transform->GetRotation();
		isMove = true;
	}
}

void DebugCamera::LastUpdate()
{
	Matrix local = transform->GetLocal();
	for (int i = 0; i < 16; i++)
	{
		rendering->World[i] = local._m[i];
	}
}
