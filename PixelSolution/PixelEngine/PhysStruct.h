#pragma once

struct PhysBox2D
{
	float CenterX;
	float CenterY;
	float OffsetX;
	float OffsetY;
};

struct PhysCircle2D
{
	float CenterX;
	float CenterY;
	float Radius;
};

struct PhysCollider
{
	std::string Key;
	int colliderType;
	union
	{
		PhysBox2D box2D;
		PhysCircle2D circle2D;
	};
};

struct PhysRigidbody
{
	bool Active = false;
	float Gravity = 1.0f;
	float Restitution = 0.0f;
	float Friction = 0.2f;
	float LinearDamping = 0.05f;
	bool AutoSleep = true;
	bool Lock[6] = {};
	int MotionType = 2;
	bool Sensor = false;

};