#include "pch.h"
#include "PMatrix.h"
#include "Type/PVector3.h"
#include "Math.h"
PMatrix::PMatrix()
{
	for (int i = 0; i < 16; i++)
	{
		switch (i)
		{
		case 0:
		case 5:
		case 10:
		case 15:
			_m[i] = 1.0f;
			break;
		default:
			_m[i] = 0.0f;
			break;
		}
	}
}

PMatrix::~PMatrix()
{

}
const float DEG2RAD = 3.141592f / 180.0f;
PMatrix PMatrix::Identity() 
{
	PMatrix res;
	res._m[0] = 1.0f; res._m[5] = 1.0f; res._m[10] = 1.0f; res._m[15] = 1.0f;
	return res;
}

// 정적 이동 행렬 생성
PMatrix PMatrix::CreateTranslation(const PVector3& Position) {
	PMatrix res = Identity();
	res._m[12] = Position.X; // Row-major 기준 마지막 행의 X
	res._m[13] = Position.Y; // Row-major 기준 마지막 행의 Y
	res._m[14] = Position.Z; // Row-major 기준 마지막 행의 Z
	return res;
}

// 정적 크기 행렬 생성
PMatrix PMatrix::CreateScale(const PVector3& Scale) {
	PMatrix res = Identity();
	res._m[0] = Scale.X;
	res._m[5] = Scale.Y;
	res._m[10] = Scale.Z;
	return res;
}

PMatrix PMatrix::CreateRotationRoll(float Roll)
{
	PMatrix res = Identity(); // 기본 단위 행렬
	float rad = Roll * DEG2RAD;
	float s = sinf(rad);
	float c = cosf(rad);

	res._m[0] = c;  res._m[1] = -s;
	res._m[4] = s;  res._m[5] = c;
	return res;
}

// X축 회전 (Pitch)
PMatrix PMatrix::CreateRotationPitch(float Pitch)
{
	PMatrix res = Identity();
	float rad = Pitch * DEG2RAD;
	float s = sinf(rad);
	float c = cosf(rad);

	res._m[5] = c;  res._m[6] = -s;
	res._m[9] = s;  res._m[10] = c;
	return res;
}

// Y축 회전 (Yaw)
PMatrix PMatrix::CreateRotationYaw(float Yaw)
{
	PMatrix res = Identity();
	float rad = Yaw * DEG2RAD;
	float s = sinf(rad);
	float c = cosf(rad);

	res._m[0] = c;  res._m[2] = s;
	res._m[8] = -s; res._m[10] = c;
	return res;
}

bool PMatrix::PMatrix::Invert(PMatrix& OutMatrix)
{
	double inv[16], det;
	int i;

	inv[0] = _m[5] * _m[10] * _m[15] -
		_m[5] * _m[11] * _m[14] -
		_m[9] * _m[6] * _m[15] +
		_m[9] * _m[7] * _m[14] +
		_m[13] * _m[6] * _m[11] -
		_m[13] * _m[7] * _m[10];

	inv[4] = -_m[4] * _m[10] * _m[15] +
		_m[4] * _m[11] * _m[14] +
		_m[8] * _m[6] * _m[15] -
		_m[8] * _m[7] * _m[14] -
		_m[12] * _m[6] * _m[11] +
		_m[12] * _m[7] * _m[10];

	inv[8] = _m[4] * _m[9] * _m[15] -
		_m[4] * _m[11] * _m[13] -
		_m[8] * _m[5] * _m[15] +
		_m[8] * _m[7] * _m[13] +
		_m[12] * _m[5] * _m[11] -
		_m[12] * _m[7] * _m[9];

	inv[12] = -_m[4] * _m[9] * _m[14] +
		_m[4] * _m[10] * _m[13] +
		_m[8] * _m[5] * _m[14] -
		_m[8] * _m[6] * _m[13] -
		_m[12] * _m[5] * _m[10] +
		_m[12] * _m[6] * _m[9];

	inv[1] = -_m[1] * _m[10] * _m[15] +
		_m[1] * _m[11] * _m[14] +
		_m[9] * _m[2] * _m[15] -
		_m[9] * _m[3] * _m[14] -
		_m[13] * _m[2] * _m[11] +
		_m[13] * _m[3] * _m[10];

	inv[5] = _m[0] * _m[10] * _m[15] -
		_m[0] * _m[11] * _m[14] -
		_m[8] * _m[2] * _m[15] +
		_m[8] * _m[3] * _m[14] +
		_m[12] * _m[2] * _m[11] -
		_m[12] * _m[3] * _m[10];

	inv[9] = -_m[0] * _m[9] * _m[15] +
		_m[0] * _m[11] * _m[13] +
		_m[8] * _m[1] * _m[15] -
		_m[8] * _m[3] * _m[13] -
		_m[12] * _m[1] * _m[11] +
		_m[12] * _m[3] * _m[9];

	inv[13] = _m[0] * _m[9] * _m[14] -
		_m[0] * _m[10] * _m[13] -
		_m[8] * _m[1] * _m[14] +
		_m[8] * _m[2] * _m[13] +
		_m[12] * _m[1] * _m[10] -
		_m[12] * _m[2] * _m[9];

	inv[2] = _m[1] * _m[6] * _m[15] -
		_m[1] * _m[7] * _m[14] -
		_m[5] * _m[2] * _m[15] +
		_m[5] * _m[3] * _m[14] +
		_m[13] * _m[2] * _m[7] -
		_m[13] * _m[3] * _m[6];

	inv[6] = -_m[0] * _m[6] * _m[15] +
		_m[0] * _m[7] * _m[14] +
		_m[4] * _m[2] * _m[15] -
		_m[4] * _m[3] * _m[14] -
		_m[12] * _m[2] * _m[7] +
		_m[12] * _m[3] * _m[6];

	inv[10] = _m[0] * _m[5] * _m[15] -
		_m[0] * _m[7] * _m[13] -
		_m[4] * _m[1] * _m[15] +
		_m[4] * _m[3] * _m[13] +
		_m[12] * _m[1] * _m[7] -
		_m[12] * _m[3] * _m[5];

	inv[14] = -_m[0] * _m[5] * _m[14] +
		_m[0] * _m[6] * _m[13] +
		_m[4] * _m[1] * _m[14] -
		_m[4] * _m[2] * _m[13] -
		_m[12] * _m[1] * _m[6] +
		_m[12] * _m[2] * _m[5];

	inv[3] = -_m[1] * _m[6] * _m[11] +
		_m[1] * _m[7] * _m[10] +
		_m[5] * _m[2] * _m[11] -
		_m[5] * _m[3] * _m[10] -
		_m[9] * _m[2] * _m[7] +
		_m[9] * _m[3] * _m[6];

	inv[7] = _m[0] * _m[6] * _m[11] -
		_m[0] * _m[7] * _m[10] -
		_m[4] * _m[2] * _m[11] +
		_m[4] * _m[3] * _m[10] +
		_m[8] * _m[2] * _m[7] -
		_m[8] * _m[3] * _m[6];

	inv[11] = -_m[0] * _m[5] * _m[11] +
		_m[0] * _m[7] * _m[9] +
		_m[4] * _m[1] * _m[11] -
		_m[4] * _m[3] * _m[9] -
		_m[8] * _m[1] * _m[7] +
		_m[8] * _m[3] * _m[5];

	inv[15] = _m[0] * _m[5] * _m[10] -
		_m[0] * _m[6] * _m[9] -
		_m[4] * _m[1] * _m[10] +
		_m[4] * _m[2] * _m[9] +
		_m[8] * _m[1] * _m[6] -
		_m[8] * _m[2] * _m[5];

	det = _m[0] * inv[0] + _m[1] * inv[4] + _m[2] * inv[8] + _m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;


	for (i = 0; i < 16; i++)
	{
		OutMatrix._m[i] = (float)inv[i] * (float)det;
	}

	return true;

}

PMatrix PMatrix::operator*(float A) const 
{
	PMatrix res = *this;
	for (int i = 0; i < 16; i++) res._m[i] *= A;
	return res;
}

// operator+= (참조 반환 필수!)
PMatrix& PMatrix::operator+=(const PMatrix& M)
{
	for (int i = 0; i < 16; i++) _m[i] += M._m[i];
	return *this;
}

PMatrix& PMatrix::operator-=(const PMatrix& M)
{
	for (int i = 0; i < 16; i++)
	{
		_m[i] -= M._m[i];
	}
	return *this; // 참조 반환 필수!
}

PMatrix PMatrix::operator*(const PMatrix& M) const
{
	PMatrix MX;
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			float sum = 0.0f;
			for (int k = 0; k < 4; k++) {
				sum += _m[row * 4 + k] * M._m[k * 4 + col];
			}
			MX._m[row * 4 + col] = sum;
		}
	}
	return MX;
}

PMatrix& PMatrix::operator=(const PMatrix& M)
{
	for (int i = 0; i < 16; i++)
	{
		_m[i] = M._m[i];
	}
	return *this;
}