#pragma once
#include <cstdint>
#include <unordered_map>

namespace ECS
{
	class Registry;

	class TweenManager
	{
	public:
		bool PingPong(Registry* registry, unsigned int id, int property, float minValue, float maxValue, float duration, bool useUnscaledTime, int ease = 0);
		bool Move(Registry* registry, unsigned int id, int property, float startValue, float endValue, float duration, bool useUnscaledTime, int ease = 0);
		bool Pause(unsigned int id, int property);
		bool Resume(unsigned int id, int property);
		bool Kill(unsigned int id, int property);
		void RemoveFromAll(unsigned int id);
		void Update(Registry* registry, float deltaTime, float unscaledDeltaTime);
		void Clear();

	private:
		bool Start(Registry* registry, unsigned int id, int property, float startValue, float endValue, float duration, bool useUnscaledTime, bool pingPong, int ease);
		struct TweenData
		{
			unsigned int ID;
			int Property;
			float StartValue;
			float EndValue;
			float Duration;
			double Elapsed = 0.0;
			bool Paused = false;
			bool UseUnscaledTime = false;
			bool PingPong = true;
			int Ease = 0;
		};
		std::unordered_map<std::uint64_t, TweenData> tweens;
	};
}
