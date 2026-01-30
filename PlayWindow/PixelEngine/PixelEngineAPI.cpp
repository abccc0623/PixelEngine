#include "PixelEngineAPI.h" 
#include "PixelEngine.h" 
#include "WindowPlatform.h"

static PixelEngine* Engine = nullptr;
bool EngineInitialize()
{
	Engine = new PixelEngine();
	Engine->Initialize();
    return true;
}

void ReleaseEngine()
{
	Engine->Release();
	delete Engine;
	Engine = nullptr;
}

void UpdateEngine()
{
	if (Engine != nullptr)
	{
		Engine->Update();
	}
}
								
void OnReSize(int X, int Y)
{

}

void GetWindowSize(int* X, int* Y)
{

}