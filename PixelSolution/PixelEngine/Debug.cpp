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

void ECS::Debug::Line(Pixel::Vector3 start, Pixel::Vector3 end, Pixel::Vector3 color)
{
	constexpr float INV_255 = 1.0f / 255.0f;
	float r = color.x * INV_255;
	float g = color.y * INV_255;
	float b = color.z * INV_255;
	DrawLine(start.x, start.y, start.z, end.x, end.y, end.z, r, g, b);
}

void ECS::Debug::Box2D(Pixel::Vector3 pos, Pixel::Vector2 size, Pixel::Vector3 color)
{
	constexpr float INV_255 = 1.0f / 255.0f;
	float r = color.x * INV_255;
	float g = color.y * INV_255;
	float b = color.z * INV_255;
	float halfX = size.x * 0.5f;
	float halfY = size.y * 0.5f;
	Pixel::Vector3 p0 = { pos.x - halfX, pos.y - halfY, pos.z };
	Pixel::Vector3 p1 = { pos.x + halfX, pos.y - halfY, pos.z };
	Pixel::Vector3 p2 = { pos.x + halfX, pos.y + halfY, pos.z };
	Pixel::Vector3 p3 = { pos.x - halfX, pos.y + halfY, pos.z };
	DrawLine(p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, r, g, b);
	DrawLine(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, r, g, b);
	DrawLine(p2.x, p2.y, p2.z, p3.x, p3.y, p3.z, r, g, b);
	DrawLine(p3.x, p3.y, p3.z, p0.x, p0.y, p0.z, r, g, b);
}

void ECS::Debug::Sphere2D(Pixel::Vector3 position, float radius, Pixel::Vector3 color)
{
	constexpr float INV_255 = 1.0f / 255.0f;
	float r = color.x * INV_255;
	float g = color.y * INV_255;
	float b = color.z * INV_255;
	DrawCircle2D(position.x, position.y, position.z, radius, r, g, b);
}