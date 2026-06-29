#include "pch.h"
#include "Debug.h"
#include "PixelGraphicsAPI.h"

void ECS::Debug::Log(const char* log)
{
	std::string logSTR(log);
	PixelLog::Info(logSTR);
}

void ECS::Debug::LogError(const char* log)
{
	std::string logSTR(log);
	PixelLog::Error(logSTR);
}

void ECS::Debug::LogWarning(const char* log)
{
	std::string logSTR(log);
	PixelLog::Warn(logSTR);
}



float To01(float value)
{
	constexpr float INV_255 = 1.0f / 255.0f;
	return std::clamp(value * INV_255, 0.0f, 1.0f);
}

void ECS::Debug::Line(float startX, float startY, float startZ, float endX, float endY, float endZ, float r, float g, float b)
{
	constexpr float INV_255 = 1.0f / 255.0f;
	DrawLine(startX, startY, startZ, endX, endY, endZ, r * INV_255, g * INV_255, b * INV_255);
}

void ECS::Debug::Box2D(float x, float y, float z, float width, float height, float r, float g, float b)
{
	constexpr float INV_255 = 1.0f / 255.0f;
	float colorR = r * INV_255;
	float colorG = g * INV_255;
	float colorB = b * INV_255;
	float halfX = width * 0.5f;
	float halfY = height * 0.5f;
	float left = x - halfX;
	float right = x + halfX;
	float bottom = y - halfY;
	float top = y + halfY;
	DrawLine(left, bottom, z, right, bottom, z, colorR, colorG, colorB);
	DrawLine(right, bottom, z, right, top, z, colorR, colorG, colorB);
	DrawLine(right, top, z, left, top, z, colorR, colorG, colorB);
	DrawLine(left, top, z, left, bottom, z, colorR, colorG, colorB);
}

void ECS::Debug::Sphere2D(float x, float y, float z, float radius, float r, float g, float b)
{
	constexpr float INV_255 = 1.0f / 255.0f;
	DrawCircle2D(x, y, z, radius, r * INV_255, g * INV_255, b * INV_255);
}
