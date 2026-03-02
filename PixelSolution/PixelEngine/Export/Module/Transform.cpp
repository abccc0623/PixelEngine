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
	IsChange_Scale(true)
{
	
}
Transform::~Transform()
{

}


void Transform::Start()
{
	WorldMatrixUpdate();
}

std::string Transform::Save()
{
	nlohmann::ordered_json j;
	j["Type"] = type;
	j["Position"] = nlohmann::ordered_json::array();
	j["Position"].push_back(meta.Position.X);
	j["Position"].push_back(meta.Position.Y);
	j["Position"].push_back(meta.Position.Z);
	j["Rotation"] = nlohmann::ordered_json::array();
	j["Rotation"].push_back(meta.Rotation.X);
	j["Rotation"].push_back(meta.Rotation.Y);
	j["Rotation"].push_back(meta.Rotation.Z);
	j["Scale"] = nlohmann::ordered_json::array();
	j["Scale"].push_back(meta.Scale.X);
	j["Scale"].push_back(meta.Scale.Y);
	j["Scale"].push_back(meta.Scale.Z);
	return j.dump(4);
}

void Transform::MatrixUpdate()
{
	WorldMatrixUpdate();
}

void Transform::SetPosition(const PVector3& mPos)
{
	IsChange_Postion = true;
	meta.Position = mPos;
}

void Transform::SetPosition(float x, float y, float z)
{
	IsChange_Postion = true;
	meta.Position.X = x;
	meta.Position.Y = y;
	meta.Position.Z = z;
}

void Transform::SetPosition(int x, int y, int z)
{
	IsChange_Postion = true;
	meta.Position.X = (float)x;
	meta.Position.Y = (float)y;
	meta.Position.Z = (float)z;
}

const PVector3& Transform::GetPosition()
{
	return meta.Position;
}

void Transform::SetRotation(const PVector3& mRot)
{
	IsChange_Rotation = true;
	meta.Rotation = mRot;
}



void Transform::SetRotation(float x, float y, float z)
{
	IsChange_Rotation = true;
	meta.Rotation.X = x;
	meta.Rotation.Y = y;
	meta.Rotation.Z = z;
}

void Transform::SetRotation(int x, int y, int z)
{
	IsChange_Rotation = true;
	meta.Rotation.X = (float)x;
	meta.Rotation.Y = (float)y;
	meta.Rotation.Z = (float)z;
}

void Transform::SetScale(const PVector3& mScale)
{
	IsChange_Scale = true;
	meta.Scale = mScale;
}

void Transform::SetScale(float x, float y, float z)
{
	IsChange_Scale = true;
	meta.Scale.X = x;
	meta.Scale.Y = y;
	meta.Scale.Z = z;
}

void Transform::SetScale(int x, int y, int z)
{
	IsChange_Scale = true;
	meta.Scale.X = (float)x;
	meta.Scale.Y = (float)y;
	meta.Scale.Z = (float)z;
}

void Transform::AddPosition(const PVector3& mPos)
{
	IsChange_Postion = true;
	meta.Position += mPos;
}

void Transform::AddPosition(float x, float y, float z)
{
	IsChange_Postion = true;
	meta.Position.X += x;
	meta.Position.Y += y;
	meta.Position.Z += z;
}

void Transform::AddPosition(int x, int y, int z)
{
	IsChange_Postion = true;
	meta.Position.X += (float)x;
	meta.Position.Y += (float)y;
	meta.Position.Z += (float)z;
}

void Transform::AddRotation(const PVector3& mRot)
{
	IsChange_Rotation = true;
	meta.Rotation += mRot;
}

void Transform::AddRotation(float x, float y, float z)
{
	IsChange_Rotation = true;
	meta.Rotation.X += x;
	meta.Rotation.Y += y;
	meta.Rotation.Z += z;
}

void Transform::AddRotation(int x, int y, int z)
{
	IsChange_Rotation = true;
	meta.Rotation.X += (float)x;
	meta.Rotation.Y += (float)y;
	meta.Rotation.Z += (float)z;
}

const PVector3& Transform::GetRotation()
{
	return meta.Rotation;
}

void Transform::AddScale(const PVector3& mScale)
{
	IsChange_Scale = true;
	meta.Scale += mScale;
}

void Transform::AddScale(float x, float y, float z)
{
	IsChange_Scale = true;
	meta.Scale.X += x;
	meta.Scale.Y += y;
	meta.Scale.Z += z;
}

void Transform::AddScale(int x, int y, int z)
{
	IsChange_Scale = true;
	meta.Scale.X += (float)x;
	meta.Scale.Y += (float)y;
	meta.Scale.Z += (float)z;
}

const PVector3& Transform::GetScale()
{
	return meta.Scale;
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
	if (IsChange_Postion == true)
	{
		Position_Matrix = Position_Matrix.CreateTranslation(meta.Position);
	}

	if (IsChange_Rotation == true)
	{
		Rotation_Pitch = Rotation_Pitch.CreateRotationPitch(meta.Rotation.X);
		Rotation_Yaw  = Rotation_Yaw.CreateRotationYaw(meta.Rotation.Y);
		Rotation_Roll = Rotation_Roll.CreateRotationRoll(meta.Rotation.Z);

		Rotation_Matrix = Rotation_Pitch * Rotation_Yaw * Rotation_Roll;
	}

	if (IsChange_Scale == true)
	{
		Scale_Matrix = Scale_Matrix.CreateScale(meta.Scale);
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
			Local_Matrix._m[MATRIX_LOCAL::POX_X] = -PVector3::Dot(meta.Position, Local_Right);
			Local_Matrix._m[MATRIX_LOCAL::POX_Y] = -PVector3::Dot(meta.Position, Local_Up);
			Local_Matrix._m[MATRIX_LOCAL::POX_Z] = -PVector3::Dot(meta.Position, Local_Look);
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

std::string Transform::RegisterLua()
{
	//auto state = GetLuaState();
	//state->new_usertype<PVector3>("PVector3",
	//	"x", &PVector3::X,
	//	"y", &PVector3::Y,
	//	"z", &PVector3::Z
	//);
	//
	//state->new_usertype<Transform>("Transform",
	//	sol::base_classes, sol::bases<Module, PixelObject>(),
	//	"SetPosition", [](Transform& obj, float x, float y, float z) {obj.SetPosition(x, y, z); },
	//	"AddPosition", [](Transform& obj, float x, float y, float z) {obj.AddPosition(x, y, z); },
	//	"GetPosition", [](Transform& obj) { return obj.Position;},
	//	"SetRotation", [](Transform& obj, float x, float y, float z) {obj.SetRotation(x, y, z); },
	//	"AddRotation", [](Transform& obj, float x, float y, float z) {obj.AddRotation(x, y, z); },
	//	"GetRotation", [](Transform& obj) { return obj.Rotation;},
	//	"SetScale", [](Transform& obj, float x, float y, float z) {obj.SetScale(x, y, z); },
	//	"AddScale", [](Transform& obj, float x, float y, float z) {obj.AddScale(x, y, z); },
	//	"GetScale", [](Transform& obj) { return obj.Scale;}
	//);
	//
	//std::string main = "";
	//{
	//	main += BindManager::ExportLuaAPIHeader<Transform>();
	//	auto func1 = static_cast<void(Transform::*)(float, float, float)>(&Transform::SetPosition);
	//	auto func2 = static_cast<void(Transform::*)(float, float, float)>(&Transform::AddPosition);
	//	auto func3 = static_cast<PVector3&(Transform::*)()>(&Transform::GetPosition);
	//	main += BindManager::ExportLuaAPIFromFunc("SetPosition", func1, "x, y, z");
	//	main += BindManager::ExportLuaAPIFromFunc("AddPosition", func2, "x, y, z");
	//	main += BindManager::ExportLuaAPIFromFunc("GetPosition", func3,"");
	//}
	//
	//{
	//	auto func1 = static_cast<void(Transform::*)(float, float, float)>(&Transform::SetRotation);
	//	auto func2 = static_cast<void(Transform::*)(float, float, float)>(&Transform::AddRotation);
	//	auto func3 = static_cast<PVector3 & (Transform::*)()>(&Transform::GetRotation);
	//	main += BindManager::ExportLuaAPIFromFunc("SetRotation", func1, "x, y, z");
	//	main += BindManager::ExportLuaAPIFromFunc("AddRotation", func2, "x, y, z");
	//	main += BindManager::ExportLuaAPIFromFunc("GetRotation", func3, "");
	//}
	//
	//{
	//	auto func1 = static_cast<void(Transform::*)(float, float, float)>(&Transform::SetScale);
	//	auto func2 = static_cast<void(Transform::*)(float, float, float)>(&Transform::AddScale);
	//	auto func3 = static_cast<PVector3 & (Transform::*)()>(&Transform::GetScale);
	//	main += BindManager::ExportLuaAPIFromFunc("SetScale", func1, "x, y, z");
	//	main += BindManager::ExportLuaAPIFromFunc("AddScale", func2, "x, y, z");
	//	main += BindManager::ExportLuaAPIFromFunc("GetScale", func3, "");
	//}
	return "";
}
