#include "pch.h"
#include "PVector3.h"
#include <cmath>

PVector3 PVector3_Normalize(PVector3 vector3)
{
	const float lengthSquared = vector3.x * vector3.x
		+ vector3.y * vector3.y
		+ vector3.z * vector3.z;

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

float PVector3_Length(PVector3 vector3)
{
	return std::sqrt(vector3.x * vector3.x
		+ vector3.y * vector3.y
		+ vector3.z * vector3.z);
}

float PVector3_Distance(PVector3 from, PVector3 to)
{
	const float x = to.x - from.x;
	const float y = to.y - from.y;
	const float z = to.z - from.z;
	return std::sqrt(x * x + y * y + z * z);
}

float PVector3_Dot(PVector3 lhs, PVector3 rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

PVector3 PVector3_Cross(PVector3 lhs, PVector3 rhs)
{
	return PVector3(
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x);
}

PVector3 PVector3_Direction(PVector3 from, PVector3 to)
{
	return PVector3_Normalize(PVector3(
		to.x - from.x,
		to.y - from.y,
		to.z - from.z));
}

PVector3 PVector3_Lerp(PVector3 from, PVector3 to, float amount)
{
	return PVector3(
		from.x + (to.x - from.x) * amount,
		from.y + (to.y - from.y) * amount,
		from.z + (to.z - from.z) * amount);
}

bool PVector3_ApproximatelyEquals(PVector3 lhs, PVector3 rhs, float epsilon)
{
	if (epsilon < 0.0f)
	{
		epsilon = -epsilon;
	}

	return std::fabs(lhs.x - rhs.x) <= epsilon
		&& std::fabs(lhs.y - rhs.y) <= epsilon
		&& std::fabs(lhs.z - rhs.z) <= epsilon;
}

PVector3 PVector3_Create(float x, float y, float z)
{
	return PVector3(x, y, z);
}
