#include "pch.h"
#include "Rigidbody2D.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "PhysManager.h"

extern PixelEngine* Engine;
void* ECS::Rigidbody2D::AddComponent(unsigned int id)
{
	auto registry = GetRegistry();
	registry->Add<Rigidbody2DData>(id);
	auto data = registry->Get<Rigidbody2DData>(id);
	return data;
}
void* ECS::Rigidbody2D::GetComponent(unsigned int id)
{
	auto registry = GetRegistry();
	Rigidbody2DData* data = registry->Get<Rigidbody2DData>(id);
	if (data == nullptr)
	{
		PixelLog::Error("[Rigidbody2D][GetComponent] Not Find Component");
	}
	return data;
}
bool ECS::Rigidbody2D::HasComponent(unsigned int id)
{
	auto registry = GetRegistry();
	Rigidbody2DData* data = registry->Get<Rigidbody2DData>(id);
	if (data == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}
std::string ECS::Rigidbody2D::BindJit()
{
	std::string jit = R"(
	---@alias MotionType
	---| 0 # Static (고정된 벽, 바닥 등)
	---| 1 # Kinematic (스크립트로 물리 법칙을 무시하고 움직이는 발판 등)
	---| 2 # Dynamic (중력과 충돌의 영향을 받는 일반적인 동적 객체)

	---@class Rigidbody2DData
	---@field type MotionType          # 물체의 물리적 성격 (Static, Kinematic, Dynamic)
	---@field active boolean           # 활성화 여부
	---@field kinematic boolean        # 키네마틱 모드 활성화 여부
	---@field autoSleep boolean        # 움직임이 없을 때 자동으로 수면(최적화) 상태로 전환할지 여부
	---@field sensor boolean           # 트리거(충돌 연산은 안 하고 감지만 함) 여부
	---@field gravity number           # 이 물체에 적용될 중력 배율
	---@field restitution number       # 탄성 계수 (통통 튀는 정도, 0.0 ~ 1.0)
	---@field friction number          # 마찰 계수 (미끄러지는 정도)
	---@field linearDamping number     # 공기 저항 (속도 감속 비율)
	---@field velocity Vector3         # 현재 선속도 (우리가 앞서 정의한 Vector3 클래스와 연동됩니다)
	---@field impulse Vector3          # 외부에서 가해진 1회성 충격량
	---@field force Vector3            # 외부에서 지속적으로 가해지는 힘
ffi.cdef[[
	typedef enum {
        Static = 0,
		Kinematic = 1,
        Dynamic = 2,
    } MotionType;

	typedef struct 
	{ 
		MotionType type;
		bool active;
		bool kinematic;
		bool autoSleep;
		bool sensor;
		
		float gravity;
		float restitution;
		float friction;
		float linearDamping;

		Vector3 velocity;
		Vector3 impulse;
		Vector3 force;

	} Rigidbody2DData;
]]
    )";
	return jit;
}

void ECS::Rigidbody2D::SetLayer(unsigned int id, const char* name)
{
	auto registry = GetRegistry();
	if (registry->Has<Rigidbody2DData>(id))
	{
		auto data = registry->Get<Rigidbody2DData>(id);
		data->layer = name;
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetLayer] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetPosition(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	if (registry->Has<Rigidbody2DData>(id) == false)
	{
		registry->Add<Rigidbody2DData>(id);
	}
	if (registry->Has<TransformData>(id))
	{
		registry->Add<TransformData>(id);
	}

	auto transformData = registry->Get<TransformData>(id);
	auto rigidbodyData = registry->Get<Rigidbody2DData>(id);
	if (transformData != nullptr)
	{
		transformData->position.x = x;
		transformData->position.y = y;
		transformData->position.z = z;
	}

	if (rigidbodyData != nullptr)
	{
		auto phys = Engine->GetFactory<PhysManager>();
		phys->SetPosition(JPH::BodyID(rigidbodyData->bodyID), x, y, z, true);
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetPosition] Not Find Component");
	}
}

void ECS::Rigidbody2D::SetRotation(unsigned int id, float x, float y, float z)
{
	auto registry = GetRegistry();
	if (registry->Has<Rigidbody2DData>(id) == false)
	{
		registry->Add<Rigidbody2DData>(id);
	}
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		auto phys = Engine->GetFactory<PhysManager>();
		phys->SetRotation(JPH::BodyID(data->bodyID), x, y, z, true);
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][SetRotation] Not Find Component");
	}
}


void ECS::Rigidbody2D::LockPosition(unsigned int id, bool x, bool y, bool z)
{
	auto registry = GetRegistry();
	registry->Add<Rigidbody2DData>(id);
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		if (data->IsCreate)
		{

		}
		else
		{

		}
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][LockPosition] Not Find Component");
	}
}

void ECS::Rigidbody2D::LockRotation(unsigned int id, bool x, bool y, bool z)
{
	auto registry = GetRegistry();
	registry->Add<Rigidbody2DData>(id);
	auto data = registry->Get<Rigidbody2DData>(id);
	if (data != nullptr)
	{
		if (data->IsCreate)
		{

		}
		else
		{

		}
	}
	else
	{
		PixelLog::Error("[Rigidbody2D][LockRotation] Not Find Component");
	}
}
