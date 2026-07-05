#include "pch.h"
#include "Input.h"
#include "PixelEngine.h"
#include "KeyInputManager.h"

extern PixelEngine* Engine;
bool Input_Key(char number)
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return input->GetKey(number);
}

bool Input_KeyDown(char number)
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return input->GetKeyDown(number);
}

bool Input_KeyUp(char number)
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return input->GetKeyUp(number);
}

int Input_GetMousePositionX()
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return input->MousePosition_X();
}

int Input_GetMousePositionY()
{
	auto input = Engine->GetFactory<KeyInputManager>();
	return input->MousePosition_Y();
}
