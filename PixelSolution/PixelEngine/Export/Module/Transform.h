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
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);


	void MatrixUpdate() override;
	void Start() override;

	PMatrix& GetWorldMatrix();
	PVector3 GetLookVector();
	PVector3 GetRightVector();
	PVector3 GetUpVector();
	PMatrix	GetLocal();
	PVector3 Position;
	PVector3 Rotation;
	PVector3 Scale;
private:
	PMatrix World_Matrix;
	PMatrix Local_Matrix;
	PMatrix Rotation_Pitch;
	PMatrix Rotation_Yaw;
	PMatrix Rotation_Roll;
	PMatrix Position_Matrix;
	PMatrix Rotation_Matrix;
	PMatrix Scale_Matrix;
	PVector3 Local_Look;
	PVector3 Local_Right;
	PVector3 Local_Up;
	void WorldMatrixUpdate();
public:
	bool IsChange_Postion;
	bool IsChange_Rotation;
	bool IsChange_Scale;

private:
	PVector3 PastPosition;
	PVector3 PastRotation;
	PVector3 PastScale;
};
