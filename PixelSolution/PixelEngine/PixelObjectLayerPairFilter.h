#pragma once
#include <Jolt/Physics/Collision/ObjectLayer.h>
namespace Layers
{
	static constexpr JPH::ObjectLayer Default = 0;
	static constexpr JPH::ObjectLayer Player = 1;
	static constexpr JPH::ObjectLayer Ground = 2;
	static constexpr JPH::ObjectLayer Layer1 = 3;
	static constexpr JPH::ObjectLayer Layer2 = 4;
	static constexpr JPH::ObjectLayer Layer3 = 5;
	static constexpr JPH::uint32 NUM_LAYERS = 6;

	static bool collisionMatrix[Layers::NUM_LAYERS][Layers::NUM_LAYERS];
}


class PixelObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
public:
	PixelObjectLayerPairFilter();
	virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2);
};

