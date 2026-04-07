#include "pch.h"
#include "DebugCamera.h"
#include "PixelGraphicsAPI.h"
#include "PixelEngineAPI.h"
#include "Core/GameObject.h"
#include "Module/Transform.h"
#include "Log.h"

DebugCamera::DebugCamera():
	isMove(false),
	moveSpeed(10),
	RotationSpeed(10),
	mLastMousePosX(0),
	mLastMousePosY(0)
{
	rendering.Type = CAMERA;
	rendering.camera.Projection = ProjectionType::Perspective;
	rendering.camera.FovY = 0.3f * 3.1415926535f;
	rendering.camera.NearZ = 0.1f;
	rendering.camera.FarZ = 4000.0f;
	rendering.camera.ZoomLevel = 1.0f;
}

DebugCamera::~DebugCamera()
{
}

void DebugCamera::Start()
{
	transform->Position += (transform->GetLookVector() * -3);
}

void DebugCamera::Update()
{
	auto DTime = GetDeltaTime();
	if (GetKey('W') == true) { transform->Position += (transform->GetLookVector() * moveSpeed * DTime); isMove = true; }
	if (GetKey('S') == true) { transform->Position += (transform->GetLookVector() * -moveSpeed * DTime); isMove = true; }
	if (GetKey('D') == true) { transform->Position += (transform->GetRightVector() * moveSpeed * DTime); isMove = true; }
	if (GetKey('A') == true) { transform->Position += (transform->GetRightVector() * -moveSpeed * DTime); isMove = true; }


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
		PVector3 Rotation = { -dy * RotationSpeed, -dx * RotationSpeed, 0 };

		transform->Rotation += Rotation;
		SetCursorPos((int)mLastMousePosX, (int)mLastMousePosY);
		isMove = true;
	}
}

void DebugCamera::LastUpdate()
{
	PMatrix local = transform->GetLocal();
	for (int i = 0; i < 16; i++)
	{
		rendering.World[i] = local._m[i];
	}
	SetRenderingData(rendering);
}
