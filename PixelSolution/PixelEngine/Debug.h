#pragma once
namespace ECS::Debug
{
	//로그 함수들
	void Log(const char* log);
	void LogError(const char* log);
	void LogWarning(const char* log);

	void Line(float startX, float startY, float startZ, float endX, float endY, float endZ, float r, float g, float b);
	void Box2D(float x, float y, float z, float width, float height, float r, float g, float b);
	void Sphere2D(float x, float y, float z, float radius, float r, float g, float b);
}