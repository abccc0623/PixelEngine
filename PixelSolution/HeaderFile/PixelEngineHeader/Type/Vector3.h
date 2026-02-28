#pragma once
class PVector3
{
public:
	PVector3();
	PVector3(const float& X, const float& Y, const float& Z);

	inline PVector3& operator =(const PVector3& Vec)
	{
		X = Vec.X; Y = Vec.Y; Z = Vec.Z;
		return *this;
	}
	inline PVector3 operator +(const PVector3& Vec) const {return PVector3(X + Vec.X, Y + Vec.Y, Z + Vec.Z);}
	inline PVector3 operator -(const PVector3& Vec) const {return PVector3(X - Vec.X, Y - Vec.Y, Z - Vec.Z);}
	inline PVector3 operator *(const PVector3& Vec) const {return PVector3(X * Vec.X, Y * Vec.Y, Z * Vec.Z);}
	inline PVector3 operator /(const PVector3& Vec) const {return PVector3(X / Vec.X, Y / Vec.Y, Z / Vec.Z);}

	inline PVector3 operator =(float f) { X = f, Y = f, Z = f; return *this; };
	inline PVector3 operator +(float f) { X += f, Y += f, Z += f; return *this; };
	inline PVector3 operator -(float f) { X -= f, Y -= f, Z -= f; return *this; };
	inline PVector3 operator *(float f) { X *= f, Y *= f, Z *= f; return *this; };
	inline PVector3 operator /(float f) { X /= f, Y /= f, Z /= f; return *this; };

	inline PVector3& operator +=(PVector3 Vec) { X += Vec.X; Y += Vec.Y; Z += Vec.Z;  return *this; };
	inline PVector3& operator -=(PVector3 Vec) { X -= Vec.X; Y -= Vec.Y; Z -= Vec.Z;  return *this; };
	inline PVector3& operator *=(PVector3 Vec) { X *= Vec.X; Y *= Vec.Y; Z *= Vec.Z;  return *this; };
	inline PVector3& operator /=(PVector3 Vec) { X /= Vec.X; Y /= Vec.Y; Z /= Vec.Z;  return *this; };

	///계산 함수
	/** 내적 */
	static float Dot(const PVector3& V1, const PVector3 V2);
	float Dot(const PVector3& V1);

	/** 외적 */
	static PVector3 Cross(const PVector3& V1, const PVector3& V2);
	PVector3 Cross(const PVector3& V1);

	/** 정규화 */
	PVector3 Normalize();
	static PVector3 Normalize(const PVector3 V1);

	static float Distance(const PVector3& Vec1, const PVector3& Vec2);
	float Distance();
	static PVector3 Lerp(const PVector3& v1, const PVector3& v2, float t);

	static const PVector3 Zero;
	static const PVector3 One;
	static const PVector3 Up;
	static const PVector3 Down;
	static const PVector3 Right;
	static const PVector3 Left;
	static const PVector3 Front;
	static const PVector3 Back;

	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;
};
