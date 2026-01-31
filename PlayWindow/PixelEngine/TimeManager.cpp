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

void TimeManager::Update()
{
    TimePoint currentTime = Clock::now();

    std::chrono::duration<float> duration = currentTime - _prevTime;
    deltaTime = duration.count();

    _prevTime = currentTime;

    // --- FPS 계산 (1초마다 갱신) ---
    frameCount++;
    frameTime += deltaTime;

    if (frameTime >= 1.0f) {
        fps = frameCount;
        frameCount = 0;
        frameTime = 0.0f;
    }
}


void TimeManager::Release()
{

}
