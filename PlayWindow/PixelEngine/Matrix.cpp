#include "Matrix.h"
#include "Vector3.h"
#include "Math.h"
Matrix::Matrix()
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

Matrix::~Matrix()
{

}

Matrix Matrix::CreateTranslation(const Vector3& Position)
{
	_m[12] = Position.X;
	_m[13] = Position.Y;
	_m[14] = Position.Z;
	return *this;
}

Matrix Matrix::CreateScale(const Vector3& Scale)
{
	_m[0]  = Scale.X;
	_m[5]  = Scale.Y;
	_m[10] = Scale.Z;
	return *this;
}

void Matrix::CreateRotationRoll(const float& Roll)
{
	float rad = Roll * 3.141592f / 180.0f;
	_m[0] = cosf(rad);	_m[1] = -sinf(rad);	_m[2] = 0;	_m[3] = 0;
	_m[4] = sinf(rad);	_m[5] = cosf(rad);	_m[6] = 0;	_m[7] = 0;
	_m[8] = 0;			_m[9] = 0;			_m[10] = 1;	_m[11] = 0;
	_m[12] = 0;			_m[13] = 0;			_m[14] = 0;	_m[15] = 1;
}

void Matrix::CreateRotationPitch(const float& Pitch)
{
	float rad = Pitch * 3.141592f / 180.0f;
	_m[0] = 1;	_m[1] = 0;			_m[2]  = 0;			_m[3] = 0;
	_m[4] = 0;	_m[5] = cosf(rad);	_m[6]  = -sinf(rad);_m[7] = 0;
	_m[8] = 0;	_m[9] = sinf(rad);	_m[10] = cosf(rad);	_m[11] = 0;
	_m[12] = 0;	_m[13] = 0;			_m[14] = 0;			_m[15] = 1;
}

void Matrix::CreateRotationYaw(const float& Yaw)
{
	float rad = Yaw * 3.141592f / 180.0f;
	_m[0]  = cosf(rad);		_m[1] = 0;		_m[2]  = sinf(rad);	_m[3] = 0;
	_m[4]  = 0;				_m[5] = 1;		_m[6]  = 0;			_m[7] = 0;
	_m[8]  = -sinf(rad);	_m[9] = 0;		_m[10] = cosf(rad);	_m[11] = 0;
	_m[12] = 0;				_m[13] = 0;		_m[14] = 0;			_m[15] = 1;
}

bool Matrix::Matrix::Invert(Matrix& OutMatrix)
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