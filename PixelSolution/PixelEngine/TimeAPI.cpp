#include "pch.h"
#include "TimeAPI.h"
#include "PixelEngine.h"
#include "TimeManager.h"

extern PixelEngine* Engine;

float Time_GetDeltaTime()
{
	if (Engine != nullptr)
	{
		return Engine->GetFactory<TimeManager>()->GetDeltaTime();
	}
	return 0.0f;
}

float Time_GetUnscaledDeltaTime()
{
	if (Engine != nullptr)
	{
		return Engine->GetFactory<TimeManager>()->GetUnscaledDeltaTime();
	}
	return 0.0f;
}

double Time_GetTotalTime()
{
	if (Engine != nullptr)
	{
		return Engine->GetFactory<TimeManager>()->GetTotalTime();
	}
	return 0.0;
}

int Time_GetFPS()
{
	if (Engine != nullptr)
	{
		return Engine->GetFactory<TimeManager>()->GetFPS();
	}
	return 0;
}

void Time_SetPaused(bool paused)
{
	if (Engine != nullptr)
	{
		Engine->GetFactory<TimeManager>()->SetPaused(paused);
	}
}

bool Time_IsPaused()
{
	if (Engine != nullptr)
	{
		return Engine->GetFactory<TimeManager>()->IsPaused();
	}
	return false;
}
