#include "pch.h"
#include "PVector3.h"
#include <cmath>

PVector3 PVector3_Normalize(PVector3 vector3)
{
	const float lengthSquared =
		vector3.x * vector3.x +
		vector3.y * vector3.y +
		vector3.z * vector3.z;

	if (lengthSquared <= 1.0e-12f)
	{
		return PVector3(0.0f, 0.0f, 0.0f);
	}

	const float inverseLength = 1.0f / std::sqrt(lengthSquared);
	return PVector3(
		vector3.x * inverseLength,
		vector3.y * inverseLength,
		vector3.z * inverseLength);
}

PVector3 PVector3_Create(float x, float y, float z)
{
	return PVector3(x, y, z);
}
