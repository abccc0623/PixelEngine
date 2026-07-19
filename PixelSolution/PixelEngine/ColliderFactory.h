#pragma once
#include "EngineManager.h"

struct PVector2;
struct BoxCollider2DData;
namespace JPH
{
	class Shape;
}
class ColliderFactory
{
public:
	ColliderFactory();
	~ColliderFactory();
	static JPH::ShapeRefC CreateBox2D(PVector2* size, PVector2* center);
	static JPH::ShapeRefC CreateCircle2D(float radius, PVector2* center);
private:
	static size_t  GetColliderKey(PVector2* size, PVector2* center);
	static size_t  GetColliderKey(float radius, PVector2* center);
	static std::unordered_map<size_t, JPH::ShapeRefC> colliderMap;
};

