#include "Transform.h"
#include "GameObject.h"
//#include "PicnicGraphicsAPI.h"
#include <iostream>
Transform::Transform():
	Rotation_Pitch(),
	Rotation_Yaw(),
	Rotation_Roll(),
	Position_Matrix(),
	Rotation_Matrix(),
	Scale_Matrix(),
	World_Matrix(),
	Position(0,0,0),
	Rotation(0,0,0),
	Scale(1,1,1),
	IsChange_Postion(true),
	IsChange_Rotation(true),
	IsChange_Scale(true)
{
	//Order = TRANSFORM_UPDATE_ORDER;
}

Transform::~Transform()
{

}


void Transform::Start()
{
	std::cout << GetClassNameString() << std::endl;
	WorldMatrixUpdate();
}

void Transform::MatrixUpdate()
{
	WorldMatrixUpdate();
}

void Transform::SetPosition(Vector3& mPos)
{
	IsChange_Postion = true;
	Position = mPos;
}

void Transform::SetPosition(float x, float y, float z)
{
	IsChange_Postion = true;
	Position.X = x;
	Position.Y = y;
	Position.Z = z;
}

void Transform::SetPosition(int x, int y, int z)
{
	IsChange_Postion = true;
	Position.X = (float)x;
	Position.Y = (float)y;
	Position.Z = (float)z;
}

Vector3& Transform::GetPosition()
{
	return Position;
}

void Transform::SetPosition(Vector3&& mPos)
{
	IsChange_Postion = true;
	Position = mPos;
}

void Transform::SetRotation(Vector3& mRot)
{
	IsChange_Rotation = true;
	Rotation = mRot;
}

void Transform::SetRotation(Vector3&& mRot)
{
	IsChange_Rotation = true;
	Rotation = mRot;
}

void Transform::SetRotation(float x, float y, float z)
{
	IsChange_Rotation = true;
	Rotation.X = x;
	Rotation.Y = y;
	Rotation.Z = z;
}

void Transform::SetRotation(int x, int y, int z)
{
	IsChange_Rotation = true;
	Rotation.X = (float)x;
	Rotation.Y = (float)y;
	Rotation.Z = (float)z;
}

void Transform::SetScale(Vector3& mScale)
{
	IsChange_Scale = true;
	Scale = mScale;
}

void Transform::SetScale(Vector3&& mScale)
{
	IsChange_Scale = true;
	Scale = mScale;
}

void Transform::SetScale(float x, float y, float z)
{
	IsChange_Scale = true;
	Scale.X = x;
	Scale.Y = y;
	Scale.Z = z;
}

void Transform::SetScale(int x, int y, int z)
{
	IsChange_Scale = true;
	Scale.X = (float)x;
	Scale.Y = (float)y;
	Scale.Z = (float)z;
}

void Transform::AddPosition(Vector3& mPos)
{
	IsChange_Postion = true;
	Position += mPos;
}

void Transform::AddPosition(Vector3&& mPos)
{
	IsChange_Postion = true;
	Position += mPos;
}

void Transform::AddPosition(float x, float y, float z)
{
	IsChange_Postion = true;
	Position.X += x;
	Position.Y += y;
	Position.Z += z;
}

void Transform::AddPosition(int x, int y, int z)
{
	IsChange_Postion = true;
	Position.X += (float)x;
	Position.Y += (float)y;
	Position.Z += (float)z;
}

void Transform::AddRotation(Vector3& mRot)
{
	IsChange_Rotation = true;
	Rotation += mRot;
}

void Transform::AddRotation(Vector3&& mRot)
{
	IsChange_Rotation = true;
	Rotation += mRot;
}

void Transform::AddRotation(float x, float y, float z)
{
	IsChange_Rotation = true;
	Rotation.X += x;
	Rotation.Y += y;
	Rotation.Z += z;
}

void Transform::AddRotation(int x, int y, int z)
{
	IsChange_Rotation = true;
	Rotation.X += (float)x;
	Rotation.Y += (float)y;
	Rotation.Z += (float)z;
}

Vector3& Transform::GetRotation()
{
	return Rotation;
}

void Transform::AddScale(Vector3& mScale)
{
	IsChange_Scale = true;
	Scale += mScale;
}

void Transform::AddScale(Vector3&& mScale)
{
	IsChange_Scale = true;
	Scale += mScale;
}

void Transform::AddScale(float x, float y, float z)
{
	IsChange_Scale = true;
	Scale.X += x;
	Scale.Y += y;
	Scale.Z += z;
}

void Transform::AddScale(int x, int y, int z)
{
	IsChange_Scale = true;
	Scale.X += (float)x;
	Scale.Y += (float)y;
	Scale.Z += (float)z;
}

Vector3& Transform::GetScale()
{
	return Scale;
}

Matrix& Transform::GetWorldMatrix()
{
	return World_Matrix;
}

Vector3 Transform::GetLookVector()
{
	return Local_Look;
}

Vector3 Transform::GetRightVector()
{
	return Local_Right;
}

Vector3 Transform::GetUpVector()
{
	return Local_Up;
}

Matrix Transform::GetLocal()
{
	return Local_Matrix;
}

void Transform::WorldMatrixUpdate()
{
	if (IsChange_Postion == true)
	{
		Position_Matrix.CreateTranslation(Position);
	}

	if (IsChange_Rotation == true)
	{
		Rotation_Pitch.CreateRotationPitch(Rotation.X);
		Rotation_Yaw.CreateRotationYaw(Rotation.Y);
		Rotation_Roll.CreateRotationRoll(Rotation.Z);

		Rotation_Matrix = Rotation_Pitch * Rotation_Yaw * Rotation_Roll;
	}

	if (IsChange_Scale == true)
	{
		Scale_Matrix.CreateScale(Scale);
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
			Local_Matrix._m[MATRIX_LOCAL::POX_X] = -Vector3::Dot(Position, Local_Right);
			Local_Matrix._m[MATRIX_LOCAL::POX_Y] = -Vector3::Dot(Position, Local_Up);
			Local_Matrix._m[MATRIX_LOCAL::POX_Z] = -Vector3::Dot(Position, Local_Look);
			Local_Matrix._m[15] = 1.0f;
			Local_Matrix._m[3] = 0.0f;
			Local_Matrix._m[7] = 0.0f;
			Local_Matrix._m[11] = 0.0f;

		}
		IsChange_Scale		= false;
		IsChange_Rotation	= false;
		IsChange_Postion	= false;

		//for(int i = 0; i < 16;i++)
		//{
		//	LinkObject->mRenderData->World[i] = World_Matrix._m[i];
		//}
	}
}

void Transform::RegisterLua()
{
	auto lua = GetLua();
	lua->new_usertype<Vector3>("Vector3",
		"x", &Vector3::X,
		"y", &Vector3::Y,
		"z", &Vector3::Z
	);

	lua->new_usertype<Transform>("Transform",
		sol::base_classes, sol::bases<Module, BaseModule>(),
		"SetPosition", [](Transform& obj, float x, float y, float z) {obj.SetPosition(x, y, z); },
		"AddPosition", [](Transform& obj, float x, float y, float z) {obj.AddPosition(x, y, z); },
		"SetRotation", [](Transform& obj, float x, float y, float z) {obj.SetRotation(x, y, z); },
		"AddRotation", [](Transform& obj, float x, float y, float z) {obj.AddRotation(x, y, z); },
		"GetPosition", [](Transform& obj) { return obj.Position;},
		"GetRotation", [](Transform& obj) { return obj.Rotation;},
		"GetScale", [](Transform& obj) { return obj.Scale;},
		"SetScale", [](Transform& obj, float x, float y, float z) {obj.SetScale(x, y, z); },
		"AddScale", [](Transform& obj, float x, float y, float z) {obj.AddScale(x, y, z); }
	);
}

void Transform::Update()
{
	
}
