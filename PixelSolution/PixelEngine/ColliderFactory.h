#pragma once
#include <sol/forward.hpp>
#include "PhysStruct.h"

class ColliderFactory
{
public:
	ColliderFactory();
	~ColliderFactory();
	JPH::ShapeRefC CreateBox2D(PhysCollider& collider);
	JPH::ShapeRefC CreateCircle2D(PhysCollider& collider);
	void Clear();
private:

	std::unordered_map<std::string, JPH::ShapeRefC> colliderMap;
};

