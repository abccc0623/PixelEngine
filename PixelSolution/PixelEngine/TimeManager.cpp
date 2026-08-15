#include "pch.h"
#include "TimeManager.h"

void TimeManager::Initialize()
{
	_startTime = Clock::now();
	_prevTime = Clock::now();
}

double TimeManager::GetTotalTime()
{
	std::chrono::duration<double> total = Clock::now() - _startTime;
	return total.count();
}

int TimeManager::GetFPS()
{
	return fps;
}

float TimeManager::GetDeltaTime()
{
	return deltaTime;
}

float TimeManager::GetUnscaledDeltaTime()
{
	return unscaledDeltaTime;
}

void TimeManager::SetPaused(bool value)
{
	paused = value;
	timeScale = paused ? 0.0f : 1.0f;
	if (paused)
	{
		deltaTime = 0.0f;
	}
}

bool TimeManager::IsPaused() const
{
	return paused;
}

void TimeManager::Update()
{
	TimePoint currentTime = Clock::now();

	std::chrono::duration<float> duration = currentTime - _prevTime;
	unscaledDeltaTime = duration.count();
	deltaTime = unscaledDeltaTime * timeScale;

	_prevTime = currentTime;

	// --- FPS 계산 (1초마다 갱신) ---
	frameCount++;
	frameTime += unscaledDeltaTime;

	if (frameTime >= 1.0f) {
		fps = frameCount;
		frameCount = 0;
		frameTime = 0.0f;
	}
}


void TimeManager::Release()
{

}

void TimeManager::Clear()
{
	paused = false;
	timeScale = 1.0f;
	deltaTime = 0.0f;
	unscaledDeltaTime = 0.0f;
	_startTime = Clock::now();
	_prevTime = _startTime;
}
