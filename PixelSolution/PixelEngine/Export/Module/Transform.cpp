#include "pch.h"
#include "Type/PVector3.h"
#include "Transform.h"
#include "Type/PMeta.h"
#include <json.hpp>

Transform::Transform():
	Rotation_Pitch(),
	Rotation_Yaw(),
	Rotation_Roll(),
	Position_Matrix(),
	Rotation_Matrix(),
	Scale_Matrix(),
	World_Matrix(),
	IsChange_Postion(true),
	IsChange_Rotation(true),
	IsChange_Scale(true),
	Position({0,0,0}),
	Rotation({0,0,0}),
	Scale({1,1,1})
{

}
Transform::~Transform()
{

}
void Transform::Start()
{
	WorldMatrixUpdate();
}

void Transform::MatrixUpdate()
{
	WorldMatrixUpdate();
}
PMatrix& Transform::GetWorldMatrix()
{
	return World_Matrix;
}

PVector3 Transform::GetLookVector()
{
	return Local_Look;
}

PVector3 Transform::GetRightVector()
{
	return Local_Right;
}

PVector3 Transform::GetUpVector()
{
	return Local_Up;
}

PMatrix Transform::GetLocal()
{
	return Local_Matrix;
}

void Transform::WorldMatrixUpdate()
{
	if (PastPosition != Position)
	{
		Position_Matrix = Position_Matrix.CreateTranslation(Position);
		PastPosition = Position;
		IsChange_Postion = true;
	}

	if (PastRotation != Rotation)
	{
		Rotation_Pitch = Rotation_Pitch.CreateRotationPitch(Rotation.X);
		Rotation_Yaw  = Rotation_Yaw.CreateRotationYaw(Rotation.Y);
		Rotation_Roll = Rotation_Roll.CreateRotationRoll(Rotation.Z);
		Rotation_Matrix = Rotation_Pitch * Rotation_Yaw * Rotation_Roll;
		PastRotation = Rotation;
		IsChange_Rotation = true;
	}
	if (PastScale != Scale)
	{
		Scale_Matrix = Scale_Matrix.CreateScale(Scale);
		PastScale = Scale;
		IsChange_Scale = true;
	}
	

	if (IsChange_Postion == true ||IsChange_Rotation == true || IsChange_Scale == true)
	{
		World_Matrix = Scale_Matrix * Rotation_Matrix * Position_Matrix;
		if(World_Matrix.Invert(Local_Matrix) == true)
		{
			Local_Right.X	= Local_Matrix._m[MATRIX_LOCAL::RIGHT_X];
			Local_Right.Y	= Local_Matrix._m[MATRIX_LOCAL::RIGHT_Y];
			Local_Right.Z	= Local_Matrix._m[MATRIX_LOCAL::RIGHT_Z];
			Local_Look.X	= Local_Matrix._m[MATRIX_LOCAL::LOOK_X];
			Local_Look.Y	= Local_Matrix._m[MATRIX_LOCAL::LOOK_Y];
			Local_Look.Z	= Local_Matrix._m[MATRIX_LOCAL::LOOK_Z];
			Local_Up.X		= Local_Matrix._m[MATRIX_LOCAL::UP_X];
			Local_Up.Y		= Local_Matrix._m[MATRIX_LOCAL::UP_Y];
			Local_Up.Z		= Local_Matrix._m[MATRIX_LOCAL::UP_Z];
			Local_Matrix._m[MATRIX_LOCAL::POX_X] = -PVector3::Dot(Position, Local_Right);
			Local_Matrix._m[MATRIX_LOCAL::POX_Y] = -PVector3::Dot(Position, Local_Up);
			Local_Matrix._m[MATRIX_LOCAL::POX_Z] = -PVector3::Dot(Position, Local_Look);
			Local_Matrix._m[15] = 1.0f;
			Local_Matrix._m[3] = 0.0f;
			Local_Matrix._m[7] = 0.0f;
			Local_Matrix._m[11] = 0.0f;
		}
		IsChange_Scale		= false;
		IsChange_Rotation	= false;
		IsChange_Postion	= false;
	}
}
