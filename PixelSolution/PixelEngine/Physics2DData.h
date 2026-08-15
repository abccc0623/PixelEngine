#pragma once
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <string>
struct Physics2DData
{
	enum Physics2DType
	{
		BOX,
		CIRCLE,
	};
	unsigned int thisID;
	Physics2DType colliderType = Physics2DType::BOX;
	JPH::ShapeRefC colliderRefC = nullptr;
	JPH::BodyID bodyID = JPH::BodyID();
	JPH::EMotionType Motiontype = JPH::EMotionType::Kinematic;
	bool sensor = true;
	bool active = true;

	std::string layer = "Default";

	float gravity = 1.0f;		//중력 (0.0 ~ 1.0)
	float restitution = 0.5f;	//탄성 계수(0.0 ~ 1.0)
	float friction = 1.0f;		//마찰 계수(0.0 ~ 1.0)
	float linearDamping = 0.0f;	//공기 저항. 물체가 이동할 때 매 순간 속도를 일정 비율로 깎습니다.

	bool Create = false;
};

