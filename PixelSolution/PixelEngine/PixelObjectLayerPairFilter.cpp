#include "pch.h"
#include "PixelObjectLayerPairFilter.h"

PixelObjectLayerPairFilter::PixelObjectLayerPairFilter()
{
}

bool PixelObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2)
{
	return Layers::collisionMatrix[inObject1][inObject2];
}
