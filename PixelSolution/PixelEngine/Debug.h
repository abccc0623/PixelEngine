#pragma once
namespace ECS::Debug
{
	//로그 함수들
	void Log(const char* log);
	void LogError(const char* log);
	void LogWarning(const char* log);

	void Line(Pixel::Vector3 start, Pixel::Vector3 end, Pixel::Vector3 color);
	void Box2D(Pixel::Vector3 pos, Pixel::Vector2 size, Pixel::Vector3 color);
	void Sphere2D(Pixel::Vector3 position, float radius, Pixel::Vector3 color);
}