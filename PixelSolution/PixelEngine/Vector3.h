#pragma once
class Vector3
{
public:
	Vector3();
	Vector3(const float& X, const float& Y, const float& Z);

	/// Vector3와 Vector3 계산 함수
	inline Vector3 operator =(Vector3 Vec) { X = Vec.X; Y = Vec.Y; Z = Vec.Z; return *this; };
	inline Vector3 operator +(Vector3 Vec) { X += Vec.X; Y += Vec.Y; Z += Vec.Z; return *this; };
	inline Vector3 operator -(Vector3 Vec) { X -= Vec.X; Y -= Vec.Y; Z -= Vec.Z; return *this; };
	inline Vector3 operator *(Vector3 Vec) { X *= Vec.X; Y *= Vec.Y; Z *= Vec.Z; return *this; };
	inline Vector3 operator /(Vector3 Vec) { X /= Vec.X; Y /= Vec.Y; Z /= Vec.Z; return *this; };

	inline Vector3 operator =(float f) { X = f, Y = f, Z = f; return *this; };
	inline Vector3 operator +(float f) { X += f, Y += f, Z += f; return *this; };
	inline Vector3 operator -(float f) { X -= f, Y -= f, Z -= f; return *this; };
	inline Vector3 operator *(float f) { X *= f, Y *= f, Z *= f; return *this; };
	inline Vector3 operator /(float f) { X /= f, Y /= f, Z /= f; return *this; };

	inline Vector3 operator +=(Vector3 Vec) { X += Vec.X; Y += Vec.Y; Z += Vec.Z;  return *this; };
	inline Vector3 operator -=(Vector3 Vec) { X -= Vec.X; Y -= Vec.Y; Z -= Vec.Z;  return *this; };
	inline Vector3 operator *=(Vector3 Vec) { X *= Vec.X; Y *= Vec.Y; Z *= Vec.Z;  return *this; };
	inline Vector3 operator /=(Vector3 Vec) { X /= Vec.X; Y /= Vec.Y; Z /= Vec.Z;  return *this; };

	///계산 함수
	/** 내적 */
	static float Dot(const Vector3& V1, const Vector3 V2);
	float Dot(const Vector3& V1);

	/** 외적 */
	static Vector3 Cross(const Vector3& V1, const Vector3& V2);
	Vector3 Cross(const Vector3& V1);

	/** 정규화 */
	Vector3 Normalize();
	static Vector3 Normalize(const Vector3 V1);

	///Static Function
	/** 두벡터 사이의 거리 */
	static float Distance(const Vector3& Vec1, const Vector3& Vec2);
	/** 원점으로 부터의 거리*/
	float Distance();
	/** 두벡터 사이의 보간 */
	static Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

	/** {0.0f,0.0f,0.0f,} */
	static Vector3 Zero;
	/** {1.0f,1.0f,1.0f,} */
	static Vector3 One;
	/** {0.0f,1.0f,0.0f,} */
	static Vector3 Up;
	/** {0.0f,-1.0f,0.0f,} */
	static Vector3 Down;
	/** {1.0f,0.0f,0.0f,} */
	static Vector3 Right;
	/** {-1.0f,0.0f,0.0f,} */
	static Vector3 Left;
	/** {0.0f,0.0f,1.0f,} */
	static Vector3 Front;
	/** {0.0f,0.0f,-1.0f,} */
	static Vector3 Back;

	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
};
