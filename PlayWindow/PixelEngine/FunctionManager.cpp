#include "FunctionManager.h"
#include "GameObject.h"
#include <iostream>
#include "Action.h"
#include "PixelObject.h"
#include "PixelEngine.h"
#include "Module.h"
#include "CollisionManager.h"
#include "ObjectManager.h"
extern PixelEngine* Engine;
FunctionManager::FunctionManager()
{
	awakeFunction   = std::queue<Action*>();
	startFunction   = std::queue<Action*>();
	updateFunction  = std::unordered_map<std::string, Action*>();
	matrixFunction  = std::unordered_map<std::string, Action*>();
	physicsFunction = std::unordered_map<std::string, Action*>();
	lastFunction	= std::unordered_map<std::string, Action*>();
	RemoveList		= std::queue<GameObject*>();
}

FunctionManager::~FunctionManager()
{
	Action::Release();
}

void FunctionManager::Initialize()
{
	collisionManager = Engine->GetFactory<CollisionManager>();
	objectManager = Engine->GetFactory<ObjectManager>();
}

void FunctionManager::Update()
{
	objectManager->DeleteCheck();
	Action::StartReady();

	//Awake에서 생성된 객체들은 그냥 다넣어줌
	isAwakeCall = true;
	while (awakeFunction.empty() == false)
	{
		auto a = awakeFunction.front();
		a->Play();
		awakeFunction.pop();
		Action::Delete(a);
	};
	isAwakeCall = false;

	//Start에서 생성된 객체들은 다음 프레임에 호출되어야 하기때문에 대기 큐에 잠시 넣어주고 실행
	while (nextStartFunction.empty() == false)
	{
		auto a = nextStartFunction.front();
		startFunction.push(a);
		nextStartFunction.pop();
	}
	while (startFunction.empty() == false)
	{
		auto a = startFunction.front();
		if (a->isReady == false)
		{
			nextStartFunction.push(a);
			startFunction.pop();
		}
		else
		{
			a->Play();
			startFunction.pop();
			Action::Delete(a);
		}
	};

	//업데이트 함수에는 함수를 가지고있다가 준비된 애들만 호출
	for (const auto& pair : updateFunction)  pair.second->Play();
	for (const auto& pair : matrixFunction)  pair.second->Play();
	for (const auto& pair : physicsFunction) pair.second->Play();

	//충돌 매니저 업데이트
	collisionManager->CollisionUpdate();

	//마지막 렌더링 넘기기 전 정리 업데이트
	for (const auto& pair : lastFunction)	 pair.second->Play();
}

void FunctionManager::Release()
{
	while (awakeFunction.empty() == false)
	{
		awakeFunction.pop();
	};

	while (startFunction.empty() == false)
	{
		startFunction.pop();
	};

	updateFunction.clear();
	matrixFunction.clear();
	physicsFunction.clear();
	lastFunction.clear();
	Action::Release();
}

void FunctionManager::AddFunction(GameObject* obj, PPointer<Module> targetModule, int type)
{
	obj->GetHashCode();
	targetModule->GetClassNameString();
	std::string key = std::to_string(obj->GetHashCode()) + "_" + targetModule->GetClassNameString();
	auto a = Action::Create();
	
	//Awake에서 추가된 모듈들은 함수 호출이 가능하도록 
	if (isAwakeCall){a->isReady = true;}

	switch (type)
	{
	case AWAKE_FUNCTION :
		a->Setting(obj, key, [targetModule] {targetModule->Awake(); });
		awakeFunction.push(a);
		break;
	case START_FUNCTION:
		a->Setting(obj,key, [targetModule] {targetModule->Start(); });
		startFunction.push(a);
		break;
	case UPDATE_FUNCTION:
		a->Setting(obj,key, [targetModule] {targetModule->Update(); });
		updateFunction.insert({ key, a});
		break;
	case MATRIX_UPDATE_FUNCTION:
		a->Setting(obj,key, [targetModule] {targetModule->MatrixUpdate(); });
		matrixFunction.insert({ key, a });
		break;
	case PHYSICS_UPDATE_FUNCTION:
		a->Setting(obj,key, [targetModule] {targetModule->PhysicsUpdate(); });
		physicsFunction.insert({ key, a });
		break;
	case LAST_UPDATE_FUNCTION:
		a->Setting(obj,key, [targetModule] {targetModule->LastUpdate(); });
		lastFunction.insert({ key, a });
		break;
	}
}

void FunctionManager::RemoveFunction(GameObject* obj)
{
	auto list = obj->GetModules();
	for (auto K : list)
	{
		std::string key = std::to_string(obj->GetHashCode()) + "_" + K->GetClassNameString();
		RemoveFunction(updateFunction, key);
		RemoveFunction(matrixFunction, key);
		RemoveFunction(physicsFunction, key);
		RemoveFunction(lastFunction, key);
	}
	//RemoveList.pop();
}

void FunctionManager::RemoveFunction(std::unordered_map<std::string, Action*>& remove, std::string key)
{
	auto it = remove.find(key);
	if (it != remove.end())
	{
		Action::Delete(it->second);
		remove.erase(it);
	}
}
