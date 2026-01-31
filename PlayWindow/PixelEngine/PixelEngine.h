#pragma once
typedef unsigned char byte;
class EngineManager; 
class Platform;
class KeyInputManager;
class TimeManager;

class PixelEngine
{
public:
	PixelEngine() = default;
	~PixelEngine() = default;
	void Initialize();
	void Update();
	void Release();
private:
	static constexpr int managerArraySize = 3;
	EngineManager** ManagerList = nullptr;

	Platform* targetPlatform = nullptr;
	TimeManager* timeManager = nullptr;
	KeyInputManager* keyInputManager = nullptr;
public:
	void QuitWindow();
	void ClientUpdate();
	void EngineUpdate();
	bool RunningCheck();

	bool GetKeyDown(byte number);
	bool GetKeyUp(byte number);
	bool GetKey(byte number);

	float GetDeltaTime();
	double GetTotalTime();
	int GetFPS();
};

