#pragma once
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include "PixelObjectLayerPairFilter.h"
namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer Default(0);
	static constexpr JPH::BroadPhaseLayer Static(1);
	static constexpr JPH::BroadPhaseLayer Dynamic(2);
	static constexpr JPH::BroadPhaseLayer Sensor(3);
	static constexpr JPH::uint32 NUM_LAYERS = 4;
}


class PixelBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
};


class PixelBroadPhaseLayerInterface : public JPH::BroadPhaseLayerInterface
{
public:
	PixelBroadPhaseLayerInterface();
	JPH::uint GetNumBroadPhaseLayers() const override;
	JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;
private:
	JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

