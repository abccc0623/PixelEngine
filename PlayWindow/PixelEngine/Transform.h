#pragma once
#include "Module.h"
#include "Vector3.h"
#include "Matrix.h"

class Transform : public Module
{
public:
	Transform();
	virtual ~Transform();
public:
	void MatrixUpdate() override;
	void Start() override;
public:
	void SetPosition(Vector3& mPos);
	void SetPosition(Vector3&& mPos);
	void SetPosition(float x, float y, float z);
	void SetPosition(int x, int y, int z);
	void AddPosition(Vector3& mPos);
	void AddPosition(Vector3&& mPos);
	void AddPosition(float x, float y, float z);
	void AddPosition(int x, int y, int z);
	Vector3& GetPosition();
public:
	void SetRotation(Vector3& mRot);
	void SetRotation(Vector3&& mRot);
	void SetRotation(float x, float y, float z);
	void SetRotation(int x, int y, int z);
	void AddRotation(Vector3& mRot);
	void AddRotation(Vector3&& mRot);
	void AddRotation(float x, float y, float z);
	void AddRotation(int x, int y, int z);
	Vector3& GetRotation();
public:
	void SetScale(Vector3& mScale);
	void SetScale(Vector3&& mScale);
	void SetScale(float x, float y, float z);
	void SetScale(int x, int y, int z);
	void AddScale(Vector3& mScale);
	void AddScale(Vector3&& mScale);
	void AddScale(float x, float y, float z);
	void AddScale(int x, int y, int z);
	Vector3& GetScale();
	Matrix& GetWorldMatrix();
public:
	Vector3 GetLookVector();
	Vector3 GetRightVector();
	Vector3 GetUpVector();
	Matrix	GetLocal();
	Matrix World_Matrix;
	Matrix Local_Matrix;
private:
	void WorldMatrixUpdate();
	Matrix Rotation_Pitch;
	Matrix Rotation_Yaw;
	Matrix Rotation_Roll;

	Matrix Position_Matrix;
	Matrix Rotation_Matrix;
	Matrix Scale_Matrix;

public:
	Vector3 Position;
	Vector3 Rotation;
	Vector3 Scale;

	Vector3 Local_Look;
	Vector3 Local_Right;
	Vector3 Local_Up;

	bool IsChange_Postion;
	bool IsChange_Rotation;
	bool IsChange_Scale;

	static void RegisterLua();
};
