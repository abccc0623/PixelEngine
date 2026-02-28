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
	void SetPosition(const PVector3& mPos);
	void SetPosition(float x, float y, float z);
	void SetPosition(int x, int y, int z);
	void AddPosition(const PVector3& mPos);
	void AddPosition(float x, float y, float z);
	void AddPosition(int x, int y, int z);
	const PVector3& GetPosition();
public:
	void SetRotation(const PVector3& mRot);
	void SetRotation(float x, float y, float z);
	void SetRotation(int x, int y, int z);
	void AddRotation(const PVector3& mRot);
	void AddRotation(float x, float y, float z);
	void AddRotation(int x, int y, int z);
	const PVector3& GetRotation();
public:
	void SetScale(const PVector3& mScale);
	void SetScale(float x, float y, float z);
	void SetScale(int x, int y, int z);
	void AddScale(const PVector3& mScale);
	void AddScale(float x, float y, float z);
	void AddScale(int x, int y, int z);
	const PVector3& GetScale();
	PMatrix& GetWorldMatrix();
public:
	PVector3 GetLookVector();
	PVector3 GetRightVector();
	PVector3 GetUpVector();
	PMatrix	GetLocal();
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
