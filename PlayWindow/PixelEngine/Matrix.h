#pragma once
class Vector3;

enum MATRIX_LOCAL : int
{
	RIGHT_X = 0,
	UP_X	= 1,
	LOOK_X	= 2,
	RIGHT_Y = 4,
	UP_Y	= 5,
	LOOK_Y	= 6,
	RIGHT_Z = 8,
	UP_Z	= 9,
	LOOK_Z	= 10,
	POX_X   = 12, 
	POX_Y   = 13, 
	POX_Z   = 14, 
};

class Matrix
{
public:
	Matrix();
	~Matrix();

	inline Matrix operator = (const Matrix& M);
	inline Matrix operator * (const Matrix& M);
	inline Matrix operator * (const float& Number);
	inline Matrix operator+= (const Matrix& M);
	inline Matrix operator-= (const Matrix& M);



	//위치 행렬
	Matrix CreateTranslation(const Vector3& Position);
	//크기 행렬
	Matrix CreateScale(const Vector3& Scale);
	//회전 행렬
	void CreateRotationRoll(const float& Roll);
	void CreateRotationPitch(const float& Pitch);
	void CreateRotationYaw(const float& Yaw);

	//역행렬
	bool Invert(Matrix& OutMatrix);
	

	float _m[16] = { 0.0f };
};



Matrix Matrix::operator=(const Matrix& M)
{
	for (int i = 0; i < 16; i++)
	{
		_m[i] = M._m[i];
	}
	return *this;
}

inline Matrix Matrix::operator*(const Matrix& M)
{
	Matrix MX;
	MX._m[0] = (_m[0] * M._m[0]) + (_m[1] * M._m[4]) + (_m[2] * M._m[8]) + (_m[3] * M._m[12]);
	MX._m[1] = (_m[0] * M._m[1]) + (_m[1] * M._m[5]) + (_m[2] * M._m[9]) + (_m[3] * M._m[13]);
	MX._m[2] = (_m[0] * M._m[2]) + (_m[1] * M._m[6]) + (_m[2] * M._m[10]) + (_m[3] * M._m[14]);
	MX._m[3] = 0;


	MX._m[4] = (_m[4] * M._m[0]) + (_m[5] * M._m[4]) + (_m[6] * M._m[8]) + (_m[7] * M._m[12]);
	MX._m[5] = (_m[4] * M._m[1]) + (_m[5] * M._m[5]) + (_m[6] * M._m[9]) + (_m[7] * M._m[13]);
	MX._m[6] = (_m[4] * M._m[2]) + (_m[5] * M._m[6]) + (_m[6] * M._m[10]) + (_m[7] * M._m[14]);
	MX._m[7] = 0;

	MX._m[8] = (_m[8] * M._m[0]) + (_m[9] * M._m[4]) + (_m[10] * M._m[8]) + (_m[11] * M._m[12]);
	MX._m[9] = (_m[8] * M._m[1]) + (_m[9] * M._m[5]) + (_m[10] * M._m[9]) + (_m[11] * M._m[13]);
	MX._m[10] = (_m[8] * M._m[2]) + (_m[9] * M._m[6]) + (_m[10] * M._m[10]) + (_m[11] * M._m[14]);
	MX._m[11] = 0;

	MX._m[12] = (_m[12] * M._m[0]) + (_m[13] * M._m[4]) + (_m[14] * M._m[8]) + (_m[15] * M._m[12]);
	MX._m[13] = (_m[12] * M._m[1]) + (_m[13] * M._m[5]) + (_m[14] * M._m[9]) + (_m[15] * M._m[13]);
	MX._m[14] = (_m[12] * M._m[2]) + (_m[13] * M._m[6]) + (_m[14] * M._m[10]) + (_m[15] * M._m[14]);
	MX._m[15] = 1;

	return MX;
}

inline Matrix Matrix::operator*(const float& A)
{
	for (int i = 0; i < 16; i++)
	{
		_m[i] *= A;
	}
	return *this;
}


Matrix Matrix::operator+=(const Matrix& M)
{
	for (int i = 0; i < 16; i++)
	{
		_m[i] += M._m[i];
	}
}

Matrix Matrix::operator-=(const Matrix& M)
{
	for (int i = 0; i < 16; i++)
	{
		_m[i] -= M._m[i];
	}
	return *this;
}

