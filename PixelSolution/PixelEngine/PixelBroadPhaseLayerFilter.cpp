#include "pch.h"
#include "PixelBroadPhaseLayerFilter.h"


bool PixelBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
{
	return true;
}


//섞이지 않도록----------나중에 클래스 내용이 많아지면 분리 
PixelBroadPhaseLayerInterface::PixelBroadPhaseLayerInterface()
{
	for (JPH::uint32 i = 0; i < Layers::NUM_LAYERS; ++i)
	{
		mObjectToBroadPhase[i] = BroadPhaseLayers::Default;
	}
}

JPH::uint PixelBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const
{
	return BroadPhaseLayers::NUM_LAYERS;
}

JPH::BroadPhaseLayer PixelBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
{
	return mObjectToBroadPhase[(JPH::uint32)inLayer];
}

