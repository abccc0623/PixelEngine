#pragma once
class PVector3
{
public:
	float x;
	float y;
	float z;
public:
	float Length();
	PVector3 Normalize();
	float Dot(PVector3 target);
	PVector3 Cross(PVector3 target);
	PVector3 Direction(PVector3 target);
	bool IsZero();
};

