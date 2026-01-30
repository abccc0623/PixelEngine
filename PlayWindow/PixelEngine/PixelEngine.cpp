#include "PixelEngine.h"
#include "WindowPlatform.h"
void PixelEngine::Initialize()
{
	targetPlatform = new WindowPlatform();
	targetPlatform->Initialize();
}

void PixelEngine::Update()
{
	if (targetPlatform != nullptr)
	{
		targetPlatform->Update();
	}
}

void PixelEngine::Release()
{
	if (targetPlatform != nullptr) 
	{
		delete targetPlatform;
		targetPlatform = nullptr;
	}
}
