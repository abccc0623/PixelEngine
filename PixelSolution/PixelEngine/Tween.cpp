#include "pch.h"
#include "Tween.h"
#include "TweenManager.h"
#include "PixelEngine.h"
#include "SceneManager.h"
#include "SceneObject.h"

extern PixelEngine* Engine;

namespace
{
	SceneObject* GetScene()
	{
		if (Engine == nullptr) return nullptr;
		auto manager = Engine->GetFactory<SceneManager>();
		return manager ? manager->GetNowScene() : nullptr;
	}
}

bool Tween_PingPong(unsigned int id, int property, float minValue, float maxValue, float duration, bool useUnscaledTime, int ease)
{
	auto scene = GetScene();
	return scene && scene->GetTweenManager()->PingPong(scene->GetRegistry(),id,property,minValue,maxValue,duration,useUnscaledTime,ease);
}

bool Tween_Move(unsigned int id, int property, float startValue, float endValue, float duration, bool useUnscaledTime, int ease)
{
	auto scene = GetScene();
	return scene && scene->GetTweenManager()->Move(scene->GetRegistry(),id,property,startValue,endValue,duration,useUnscaledTime,ease);
}

bool Tween_Pause(unsigned int id, int property)
{
	auto scene = GetScene();
	return scene && scene->GetTweenManager()->Pause(id,property);
}

bool Tween_Resume(unsigned int id, int property)
{
	auto scene = GetScene();
	return scene && scene->GetTweenManager()->Resume(id,property);
}

bool Tween_Kill(unsigned int id, int property)
{
	auto scene = GetScene();
	return scene && scene->GetTweenManager()->Kill(id,property);
}
