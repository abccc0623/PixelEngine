#include "KeyInputManager.h"
#include "windows.h"
void KeyInputManager::Initialize()
{
	for (int i = 0; i < arraySize; i++)
	{
		keyStates[i] = KeyState::NONE;
	}
}

void KeyInputManager::Update()
{
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
			keyStates[i] = wasPressed ? UP : NONE;
        }
	}
}

void KeyInputManager::Release()
{
	for (int i = 0; i < arraySize; i++)
	{
		keyStates[i] = KeyState::NONE;
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
