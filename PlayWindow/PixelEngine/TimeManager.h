#pragma once
#include <chrono>
#include "EngineManager.h"

class TimeManager : public EngineManager
{
public:
    TimeManager() = default;
    ~TimeManager() = default;
  

    float GetDeltaTime();
    double GetTotalTime();
    int GetFPS();

    void Update() override;
    void Initialize() override;
    void ReleaseShared() override;
private:
    using Clock = ::std::chrono::high_resolution_clock;
    using TimePoint = ::std::chrono::time_point<Clock>;

    TimePoint _startTime;
    TimePoint _prevTime;

    float deltaTime = 0.0f;

    // FPS °è»ê¿ë
    float frameTime = 0.0f;
    unsigned int frameCount = 0;
    unsigned int fps = 0;
};

