#include "pch.h"
#include "PixelObjectLayerPairFilter.h"

PixelObjectLayerPairFilter::PixelObjectLayerPairFilter()
{
}

bool PixelObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
{
	if (inObject1 >= Layers::layerCount || inObject2 >= Layers::layerCount)
	{
		return false;
	}

	return Layers::collisionMatrix[inObject1][inObject2];
}
