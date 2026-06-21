#include "pch.h"
#include "KeyInputManager.h"
#include "EventManager.h"
#include "PixelEngine.h"
#include "Type/GlobalEnum.h"
#pragma comment(lib, "User32.lib")
extern PixelEngine* Engine;
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
	if (Focus != pastFocus)
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
			//if (keyStates[i] & DOWN)
			//{
			//	EventMessage eventMessage;
			//	eventMessage.key.keyCode = i;
			//	eventMessage.key.Pressed = true;
			//	event->TriggerEvent(EventType::KeyDown, eventMessage);
			//}
		}
		else
		{
			keyStates[i] = wasPressed ? UP : KEY_NONE;
			//if (keyStates[i] & UP)
			//{
			//	EventMessage eventMessage;
			//	eventMessage.key.keyCode = i;
			//	eventMessage.key.Pressed = false;
			//	event->TriggerEvent(EventType::KeyUp, eventMessage);
			//}
		}
	}
}

void KeyInputManager::Release()
{
	for (int i = 0; i < arraySize; i++)
	{
		keyStates[i] = KeyState::KEY_NONE;
	}
}
void KeyInputManager::Clear()
{

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
