#pragma once
#include "EngineManager.h"
#include <windows.h>
enum KeyState {
	KEY_NONE	= 0,		// 0000 0000
	PRESSED = 1 << 0,   // 0001 : 현재 눌려있음
	DOWN	= 1 << 1,   // 0010 : 이번에 막 눌림
	UP		= 1 << 2,   // 0100 : 이번에 막 떨어짐
};
typedef unsigned char byte;
class KeyInputManager : public EngineManager
{
public:
	KeyInputManager() = default;
	~KeyInputManager() = default;

	void Initialize() override;
	void Update() override;
	void Release() override;

	bool GetKey(byte keyNumber);
	bool GetKeyDown(byte keyNumber);
	bool GetKeyUp(byte keyNumber);
	int MousePosition_X();
	int MousePosition_Y();

	POINT ClientCursorPos;

private:
	static constexpr int arraySize = 256;
	unsigned char keyStates[arraySize];
};

