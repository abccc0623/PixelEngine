#pragma once
#include "Core/Module.h"
#include "Type/PVector3.h"
#include "Type/PMatrix.h"

class Transform : public Module
{
public:
	Transform();
	virtual ~Transform();
public:
	void MatrixUpdate() override;
	void Start() override;
public:
	PIXEL_ENGINEDLL void SetPosition(const PVector3& mPos);
	PIXEL_ENGINEDLL void SetPosition(float x, float y, float z);
	PIXEL_ENGINEDLL void SetPosition(int x, int y, int z);
	PIXEL_ENGINEDLL void AddPosition(const PVector3& mPos);
	PIXEL_ENGINEDLL void AddPosition(float x, float y, float z);
	PIXEL_ENGINEDLL void AddPosition(int x, int y, int z);
	PIXEL_ENGINEDLL const PVector3& GetPosition();
public:
	PIXEL_ENGINEDLL void SetRotation(const PVector3& mRot);
	PIXEL_ENGINEDLL void SetRotation(float x, float y, float z);
	PIXEL_ENGINEDLL void SetRotation(int x, int y, int z);
	PIXEL_ENGINEDLL void AddRotation(const PVector3& mRot);
	PIXEL_ENGINEDLL void AddRotation(float x, float y, float z);
	PIXEL_ENGINEDLL void AddRotation(int x, int y, int z);
	PIXEL_ENGINEDLL const PVector3& GetRotation();
public:
	PIXEL_ENGINEDLL void SetScale(const PVector3& mScale);
	PIXEL_ENGINEDLL void SetScale(float x, float y, float z);
	PIXEL_ENGINEDLL void SetScale(int x, int y, int z);
	PIXEL_ENGINEDLL void AddScale(const PVector3& mScale);
	PIXEL_ENGINEDLL void AddScale(float x, float y, float z);
	PIXEL_ENGINEDLL void AddScale(int x, int y, int z);
	PIXEL_ENGINEDLL const PVector3& GetScale();
	PIXEL_ENGINEDLL PMatrix& GetWorldMatrix();
public:
	PIXEL_ENGINEDLL PVector3 GetLookVector();
	PIXEL_ENGINEDLL PVector3 GetRightVector();
	PIXEL_ENGINEDLL PVector3 GetUpVector();
	PIXEL_ENGINEDLL PMatrix	GetLocal();
	PMatrix World_Matrix;
	PMatrix Local_Matrix;
private:
	void WorldMatrixUpdate();
	PMatrix Rotation_Pitch;
	PMatrix Rotation_Yaw;
	PMatrix Rotation_Roll;

	PMatrix Position_Matrix;
	PMatrix Rotation_Matrix;
	PMatrix Scale_Matrix;

	PVector3 Position;
	PVector3 Rotation;
	PVector3 Scale;

	PVector3 Local_Look;
	PVector3 Local_Right;
	PVector3 Local_Up;
public:
	bool IsChange_Postion;
	bool IsChange_Rotation;
	bool IsChange_Scale;
	static std::string RegisterLua();
};
