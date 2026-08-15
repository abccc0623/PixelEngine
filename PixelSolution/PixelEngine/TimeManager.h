#pragma once
#include <chrono>
#include "EngineManager.h"

class TimeManager : public EngineManager
{
public:
	TimeManager() = default;
	~TimeManager() = default;


	float GetDeltaTime();
	float GetUnscaledDeltaTime();
	double GetTotalTime();
	int GetFPS();
	void SetPaused(bool value);
	bool IsPaused() const;

	void Update() override;
	void Initialize() override;
	void Release() override;
	void Clear() override;
private:
	using Clock = ::std::chrono::high_resolution_clock;
	using TimePoint = ::std::chrono::time_point<Clock>;

	TimePoint _startTime;
	TimePoint _prevTime;

	float deltaTime = 0.0f;
	float unscaledDeltaTime = 0.0f;
	float timeScale = 1.0f;
	bool paused = false;

	// FPS 계산용
	float frameTime = 0.0f;
	unsigned int frameCount = 0;
	unsigned int fps = 0;
};

