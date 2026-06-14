#pragma once
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <string>
#include <vector>
namespace Layers
{
	static constexpr JPH::ObjectLayer Default = 0;
	static constexpr JPH::ObjectLayer Player = 1;
	static constexpr JPH::ObjectLayer Ground = 2;
	static constexpr JPH::uint32 NUM_LAYERS = 32;

	inline bool collisionMatrix[Layers::NUM_LAYERS][Layers::NUM_LAYERS] = {};
	inline JPH::uint32 layerCount = 0;
	inline std::vector<std::string> layerNames;
}


class PixelObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
public:
	PixelObjectLayerPairFilter();
	bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;
};

