#include "pch.h"
#include "math.h"
#include "PVector3.h"

//Static 변수 초기화
const PVector3 PVector3::Zero	= { 0.0f,0.0f,0.0f };
const PVector3 PVector3::One	= { 1.0f,1.0f,1.0f };
const PVector3 PVector3::Up		= { 0.0f,1.0f,0.0f };
const PVector3 PVector3::Down	= { 0.0f,-1.0f,0.0f };
const PVector3 PVector3::Right	= { -1.0f,0.0f,0.0f };
const PVector3 PVector3::Left	= { 1.0f,0.0f,0.0f };
const PVector3 PVector3::Front	= { 0.0f,0.0f,1.0f };
const PVector3 PVector3::Back	= { 0.0f,0.0f,-1.0f };

//기본 생성자, 복사 생성자, 이동 생성자
PVector3::PVector3() :X(0.0f), Y(0.0f), Z(0.0f){}
PVector3::PVector3(const float& X, const float& Y, const float& Z) : X(X), Y(Y), Z(Z) {}

float PVector3::Dot(const PVector3& V1, const PVector3 V2)
{
	//벡터의 내적
	return ((V1.X * V2.X) + (V1.Y * V2.Y) + (V1.Z * V2.Z));
}

float PVector3::Dot(const PVector3& V1)
{
	//벡터의 내적
	return ((X * V1.X) + (Y * V1.Y) + (Z * V1.Z));
}

PVector3 PVector3::Cross(const PVector3& V1, const PVector3& V2)
{
	//벡터의 외적
	PVector3 mCross;
	mCross.X = (V1.Y * V2.Z) - (V1.Z * V2.Y);
	mCross.Y = (V1.Z * V2.X) - (V1.X * V2.Z);
	mCross.Z = (V1.X * V2.Y) - (V1.Y * V2.X);

	return mCross;
}

PVector3 PVector3::Cross(const PVector3& V1)
{
	//벡터의 외적
	PVector3 mCross;
	mCross.X = (Y * V1.Z) - (Z * V1.Y);
	mCross.Y = (Z * V1.X) - (X * V1.Z);
	mCross.Z = (X * V1.Y) - (Y * V1.X);

	return mCross;
}

PVector3 PVector3::Normalize()
{
	float n = float(sqrt(pow(X, 2) + pow(Y, 2) + pow(Z, 2)));
	X /= n;
	Y /= n;
	Z /= n;
	return *this;
}

PVector3 PVector3::Normalize(const PVector3 V1)
{
	PVector3 mNormalize;
	float n = float(sqrt(pow(V1.X, 2) + pow(V1.Y, 2) + pow(V1.Z, 2)));
	mNormalize.X /= n;
	mNormalize.Y /= n;
	mNormalize.Z /= n;
	return mNormalize;
}

float PVector3::Distance()
{
	return float(sqrt((X * X)  + (Y * Y) + (Z * Z)));
}

float PVector3::Distance(const PVector3& Vec1, const PVector3& Vec2)
{
	return float(sqrt(powf(Vec2.X - Vec1.X, 2) + powf(Vec2.Y - Vec1.Y, 2) + powf(Vec2.Z - Vec1.Z, 2)));
}

PVector3 PVector3::Lerp(const PVector3& v1, const PVector3& v2, float t)
{
	PVector3 LerpVector
	(
		(v1.X * (1 - t)) + (v2.X * t),
		(v1.Y * (1 - t)) + (v2.Y * t),
		(v1.Z * (1 - t)) + (v2.Z * t)
	);
	return LerpVector;
}