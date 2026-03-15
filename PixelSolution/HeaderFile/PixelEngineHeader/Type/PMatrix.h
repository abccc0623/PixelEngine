#pragma once
class PVector3;

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

class PMatrix
{
public:
	PMatrix();
	~PMatrix();

	PMatrix& operator=(const PMatrix& M);
	PMatrix operator*(const PMatrix& M) const;
	PMatrix operator*(float Number) const;
	PMatrix& operator+=(const PMatrix& M);
	PMatrix& operator-=(const PMatrix& M);

	static PMatrix Identity();
	static PMatrix CreateTranslation(const PVector3& Position);
	static PMatrix CreateScale(const PVector3& Scale);
	static PMatrix CreateRotationRoll(float Roll);
	static PMatrix CreateRotationPitch(float Pitch);
	static PMatrix CreateRotationYaw(float Yaw);

	//¿ªÇà·Ä
	bool Invert(PMatrix& OutMatrix);
	
	float _m[16] = { 0.0f };
};




