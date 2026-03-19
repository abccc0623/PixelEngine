#include "pch.h"
#include "KeyInputManager.h"
#pragma comment(lib, "User32.lib")
KeyInputManager::KeyInputManager()
{

}
KeyInputManager::~KeyInputManager()
{

}
void KeyInputManager::Initialize()
{
	for (int i = 0; i < arraySize; i++)
	{
		keyStates[i] = KeyState::KEY_NONE;
	}
}

void KeyInputManager::Update()
{
	if(Focus != pastFocus)
	{
		for (int i = 0; i < arraySize; i++)
		{
			keyStates[i] = KEY_NONE;
		}
		return;
	}
	if (Focus == false)return;

	for (int i = 0; i < arraySize; i++)
	{
		bool isCurrentPressed = (GetAsyncKeyState(i) & 0x8000) != 0;
		bool wasPressed = (keyStates[i] & PRESSED) != 0;

        if (isCurrentPressed)
        {
			keyStates[i] = wasPressed ? PRESSED : PRESSED | DOWN;
        }
        else
        {
			keyStates[i] = wasPressed ? UP : KEY_NONE;
        }
	}
}

void KeyInputManager::ReleaseShared()
{
	for (int i = 0; i < arraySize; i++)
	{
		keyStates[i] = KeyState::KEY_NONE;
	}
}

bool KeyInputManager::GetKey(byte keyNumber)
{
	return (keyStates[keyNumber] & PRESSED) ? true : false;
}

bool KeyInputManager::GetKeyDown(byte keyNumber)
{
	return (keyStates[keyNumber] & DOWN) ? true : false;
}

bool KeyInputManager::GetKeyUp(byte keyNumber)
{
	return (keyStates[keyNumber] & UP) ? true : false;
}

int KeyInputManager::MousePosition_X()
{
	GetCursorPos(&ClientCursorPos);
	return (int)ClientCursorPos.x;
}

int KeyInputManager::MousePosition_Y()
{
	GetCursorPos(&ClientCursorPos);
	return (int)ClientCursorPos.y;
}
