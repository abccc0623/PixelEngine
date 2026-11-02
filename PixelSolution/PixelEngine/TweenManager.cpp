#include "pch.h"
#include "TweenManager.h"
#include "Tween.h"
#include "Registry.h"
#include "Transform.h"
#include <cmath>

namespace
{
	bool ValidProperty(int property)
	{
		return property >= static_cast<int>(TweenProperty::PositionX) && property <= static_cast<int>(TweenProperty::ScaleZ);
	}

	std::uint64_t MakeKey(unsigned int id, int property)
	{
		return (static_cast<std::uint64_t>(id) << 32) | static_cast<std::uint32_t>(property);
	}

	float* GetTarget(TransformData* transform, int property)
	{
		if (transform == nullptr) return nullptr;
		switch (static_cast<TweenProperty>(property))
		{
		case TweenProperty::PositionX: return &transform->position.x;
		case TweenProperty::PositionY: return &transform->position.y;
		case TweenProperty::PositionZ: return &transform->position.z;
		case TweenProperty::RotationX: return &transform->rotation.x;
		case TweenProperty::RotationY: return &transform->rotation.y;
		case TweenProperty::RotationZ: return &transform->rotation.z;
		case TweenProperty::ScaleX: return &transform->scale.x;
		case TweenProperty::ScaleY: return &transform->scale.y;
		case TweenProperty::ScaleZ: return &transform->scale.z;
		default: return nullptr;
		}
	}
}

bool ECS::TweenManager::PingPong(Registry* registry, unsigned int id, int property, float minValue, float maxValue, float duration, bool useUnscaledTime, int ease)
{
	if (minValue > maxValue) return false;
	return Start(registry,id,property,minValue,maxValue,duration,useUnscaledTime,true,ease);
}

bool ECS::TweenManager::Move(Registry* registry, unsigned int id, int property, float startValue, float endValue, float duration, bool useUnscaledTime, int ease)
{
	return Start(registry,id,property,startValue,endValue,duration,useUnscaledTime,false,ease);
}

bool ECS::TweenManager::Start(Registry* registry, unsigned int id, int property, float startValue, float endValue, float duration, bool useUnscaledTime, bool pingPong, int ease)
{
	if (registry == nullptr || !ValidProperty(property) || !std::isfinite(startValue) || !std::isfinite(endValue) ||
		!std::isfinite(duration) || duration <= 0.0f || ease < 0 || ease > static_cast<int>(TweenEase::InOut))
	{
		return false;
	}
	auto transform = registry->Get<TransformData>(id);
	auto target = GetTarget(transform, property);
	if (target == nullptr) return false;
	// 객체 주소는 보관하지 않는다. 같은 ID와 속성은 처음부터 다시 시작한다.
	tweens.insert_or_assign(MakeKey(id, property), TweenData{ id,property,startValue,endValue,duration,0.0,false,useUnscaledTime,pingPong,ease });
	*target = startValue;
	transform->bitmask |= 0x01;
	return true;
}

bool ECS::TweenManager::Pause(unsigned int id, int property)
{
	if (!ValidProperty(property)) return false;
	auto found = tweens.find(MakeKey(id, property));
	if (found == tweens.end()) return false;
	found->second.Paused = true;
	return true;
}

bool ECS::TweenManager::Resume(unsigned int id, int property)
{
	if (!ValidProperty(property)) return false;
	auto found = tweens.find(MakeKey(id, property));
	if (found == tweens.end()) return false;
	found->second.Paused = false;
	return true;
}

bool ECS::TweenManager::Kill(unsigned int id, int property)
{
	return ValidProperty(property) && tweens.erase(MakeKey(id, property)) != 0;
}

void ECS::TweenManager::RemoveFromAll(unsigned int id)
{
	for (int property = static_cast<int>(TweenProperty::PositionX); property <= static_cast<int>(TweenProperty::ScaleZ); ++property)
	{
		tweens.erase(MakeKey(id, property));
	}
}

void ECS::TweenManager::Update(Registry* registry, float deltaTime, float unscaledDeltaTime)
{
	if (registry == nullptr) return;
	for (auto it = tweens.begin(); it != tweens.end();)
	{
		auto& tween = it->second;
		// 컴포넌트는 이번 갱신에만 조회하므로 저장소 변경 후에도 옛 포인터를 쓰지 않는다.
		auto transform = registry->Get<TransformData>(tween.ID);
		auto target = GetTarget(transform, tween.Property);
		if (target == nullptr)
		{
			it = tweens.erase(it);
			continue;
		}
		const float step = tween.UseUnscaledTime ? unscaledDeltaTime : deltaTime;
		if (!tween.Paused && std::isfinite(step) && step > 0.0f)
		{
			const double duration = tween.Duration;
			if (!tween.PingPong && tween.Elapsed + step >= duration)
			{
				*target = tween.EndValue;
				transform->bitmask |= 0x01;
				it = tweens.erase(it);
				continue;
			}
			if (tween.PingPong)
			{
				tween.Elapsed = std::fmod(tween.Elapsed + step, duration * 2.0);
			}
			else
			{
				tween.Elapsed += step;
			}
			const double phase = tween.Elapsed / duration;
			const double progress = phase <= 1.0 ? phase : 2.0 - phase;
			// 복귀 구간도 진행 방향 기준으로 In/Out을 적용한다.
			double t = phase <= 1.0 ? progress : 1.0 - progress;
			switch (static_cast<TweenEase>(tween.Ease))
			{
			case TweenEase::In: t = t * t; break;
			case TweenEase::Out: t = t * (2.0 - t); break;
			case TweenEase::InOut: t = t < 0.5 ? 2.0 * t * t : 1.0 - 2.0 * (1.0 - t) * (1.0 - t); break;
			default: break;
			}
			if (phase > 1.0) t = 1.0 - t;
			*target = static_cast<float>(static_cast<double>(tween.StartValue) + (static_cast<double>(tween.EndValue) - tween.StartValue) * t);
			transform->bitmask |= 0x01;
		}
		++it;
	}
}

void ECS::TweenManager::Clear()
{
	tweens.clear();
}
