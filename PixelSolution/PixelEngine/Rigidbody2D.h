#pragma once

enum MotionType
{
	Static,
	Kinematic,
	Dynamic,
};
namespace ECS::Rigidbody2D
{
	struct Rigidbody2DData
	{
		MotionType type = MotionType::Dynamic;
		bool Active = true;			//활성화 여부
		bool Kinematic = false;		//물리 법칙(중력, 마찰)은 안 따르지만, 내가 직접 코드로 위치를 옮기는 물체
		bool AutoSleep = true;		//물체가 움직이지 않을 때 계산을 멈출지 결정(최적화)
		bool Sensor = false;		//유령처럼 통과할 것인가, 아니면 물리적인 접촉 신호를 보낼 것인가 충돌은 감지
		float Gravity = 1.0f;		//중력 (0.0 ~ 1.0)
		float Restitution = 0.5f;	//탄성 계수(0.0 ~ 1.0)
		float Friction = 1.0f;		//마찰 계수(0.0 ~ 1.0)
		float LinearDamping = 0.0f;	//공기 저항. 물체가 이동할 때 매 순간 속도를 일정 비율로 깎습니다.

		Pixel::Vector3 velocity = { 0,0,0 };
		bool lockPosition[3];
		bool lockRotation[3];
		unsigned int bodyID;
		bool IsCreate = false;
	};
	void* AddComponent(unsigned int id);
	void* GetComponent(unsigned int id);
	bool HasComponent(unsigned int id);
	std::string BindJit();

	void SetPosition(unsigned int id, float x, float y, float z);
	void SetRotation(unsigned int id, float x, float y, float z);
	void AddImpulse(unsigned int id, float x, float y, float z);
	void AddForce(unsigned int id, float x, float y, float z);
	void LockPosition(unsigned int id, bool x, bool y, bool z);
	void LockRotation(unsigned int id, bool x, bool y, bool z);
}

