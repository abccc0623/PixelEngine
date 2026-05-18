#include "pch.h"
#include "Movement.h"
#include "PixelEngine.h"
#include "SceneManager.h"
#include "Type/PVector3.h"
#include "Scene.h"
extern PixelEngine* Engine;
Movement::Movement()
{
	bitmask.set(NONE);
}

Movement::~Movement()
{

}

void Movement::Start()
{
	//if (targetObject->HasModuleToEngine("Physics2D"))
	//{
	//	physics2D = static_cast<Physics2D*>(targetObject->GetModuleToEngine("Physics2D"));
	//}
}

void Movement::Update()
{
	// 1. 목적지(Destination) 결정하기
	//bool hasDestination = false;
	//PVector3 destPos;
	//
	//if (isTargetingPosition)
	//{
	//	// 좌표 기반 이동 모드일 경우
	//	destPos = targetPosition;
	//	hasDestination = true;
	//}
	//else if (target.IsValid())
	//{
	//	// 객체 추적 모드일 경우
	//	//auto k = target.Lock();
	//	//auto tr = k->GetTransform();
	//	//destPos = tr->Position;
	//	//hasDestination = true;
	//}
	//
	//// 목적지가 없으면(타겟 객체도 없고 좌표 설정도 안 됨) 리턴
	//if (!hasDestination) return;
	//
	//// 2. 거리와 방향 계산
	//auto distance = Distance(transform->Position, destPos);
	//auto direction = destPos - transform->Position;
	//
	//// 안전장치: 거리가 0일 때 Normalize()를 하면 값이 터질 수 있으므로 방어코드 추가
	//if (distance > 0.0001f) {
	//	direction = direction.Normalize();
	//}
	//else {
	//	direction = PVector3(0, 0, 0);
	//}
	//
	//// 3. 이동 및 비트마스크 처리
	//if (distance > stopDistance)
	//{
	//	if (physics2D == nullptr || physics2D->GetActive() == false)
	//	{
	//		transform->Position += direction * (GetDeltaTime() * speed);
	//	}
	//	else
	//	{
	//		PVector3 v = direction * speed;
	//		physics2D->SetVelocity(v.X, v.Y);
	//	}
	//	bitmask.set(MOVE);
	//}
	//else
	//{
	//	// 멈춰야 할 때
	//	bitmask.reset(MOVE);
	//	// [선택 사항] 좌표 이동의 경우, 도착하면 타겟팅 모드를 꺼버립니다.
	//	if (isTargetingPosition) {
	//		isTargetingPosition = false;
	//	}
	//}
	//
	//// 4. 기존 콜백 로직 유지
	//// 타겟과의 방향을 계산 (방향이 0이 아닐 때만 갱신하도록 조건 추가)
	//if (direction != lastDirection && distance > 0.0001f)
	//{
	//	lastDirection = direction;
	//	auto result = directionChange(instance, lastDirection);
	//	if (!result.valid())
	//	{
	//		sol::error err = result;
	//		std::string what = err.what();
	//		PixelLog::Error(what);
	//	}
	//}
	//
	//// 이동 중이고 START 비트가 켜져 있지 않으면
	//if (bitmask.hasAny(MOVE) == true && bitmask.hasAny(START) == false)
	//{
	//	CallFunction(started);
	//	bitmask.set(START);
	//	bitmask.reset(END);
	//}
	//else if (bitmask.hasAny(MOVE) == false && bitmask.hasAny(END) == false)
	//{
	//	// 현재 이동X, 이전에 complete 함수가 호출 안 됐다면
	//	if (bitmask.hasAny(START))
	//	{
	//		CallFunction(complete);
	//		bitmask.set(END);
	//		bitmask.reset(START);
	//	}
	//}
}


void Movement::MoveToPosition(float x, float y, float z)
{
	bitmask.reset(STOP);
	isTargetingPosition = true; // 좌표 이동 모드 켜기
	targetPosition = PVector3(x, y, z); // 목표 좌표 설정
}

//void Movement::MoveToTarget(GameObject* target)
//{
	//bitmask.reset(STOP);
	//isTargetingPosition = false;
	//auto manager = Engine->GetFactory<SceneManager>();
	//auto scene =  manager->GetNowScene();
	//this->target = scene->FindGameObjectToEngine(target);
//}

void Movement::StopMove()
{
	bitmask.set(STOP);
}

void Movement::AddCompleteCallBack(std::string functionName)
{
	//Module* targetModule = targetObject->GetModuleToEngine("LuaScript");
	//if (targetModule != nullptr)
	//{
	//	LuaScript* script = static_cast<LuaScript*>(targetModule);
	//	instance = script->Get();
	//	complete = instance[functionName];
	//}
}

void Movement::AddStartedCallBack(std::string functionName)
{
	//Module* targetModule = targetObject->GetModuleToEngine("LuaScript");
	//if (targetModule != nullptr)
	//{
	//	LuaScript* script = static_cast<LuaScript*>(targetModule);
	//	instance = script->Get();
	//	started = instance[functionName];
	//}
}

void Movement::AddDirectionCallBack(std::string functionName)
{
	//Module* targetModule = targetObject->GetModuleToEngine("LuaScript");
	//if (targetModule != nullptr)
	//{
	//	LuaScript* script = static_cast<LuaScript*>(targetModule);
	//	instance = script->Get();
	//	directionChange = instance[functionName];
	//}
}

void Movement::CallFunction(sol::function& call)
{
	if (call.valid())
	{
		auto result = call(instance);
		if (!result.valid())
		{
			sol::error err = result;
			std::string what = err.what();
			PixelLog::Error(what);
		}
	}
}
