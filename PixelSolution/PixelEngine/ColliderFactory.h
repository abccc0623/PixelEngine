#pragma once
#include "Collider2D.h"
namespace ECS::Collider2D
{
	struct Collider2DData;
}
namespace JPH
{
	class Shape;
}
class ColliderFactory
{
public:
	ColliderFactory();
	~ColliderFactory();
	JPH::ShapeRefC CreateBox2D(ECS::Collider2D::Collider2DData* collider);
	JPH::ShapeRefC CreateCircle2D(ECS::Collider2D::Collider2DData* collider);
	void Clear();
private:

	std::unordered_map<JPH::Shape*, JPH::ShapeRefC> colliderMap;
};

