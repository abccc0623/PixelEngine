#include "FunctionManager.h"
#include "GameObject.h"
#include <iostream>
#include "Action.h"

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

}

void FunctionManager::Update()
{
	Action::StartReady();

	isAwakeCall = true;
	while (awakeFunction.empty() == false)
	{
		auto a = awakeFunction.front();
		a->Play();
		awakeFunction.pop();
		Action::Delete(a);
	};
	isAwakeCall = false;

	while (startFunction.empty() == false)
	{
		auto a = startFunction.front();
		a->Play();
		startFunction.pop();
		Action::Delete(a);
	};

	for (const auto& pair : updateFunction)  pair.second->Play();
	for (const auto& pair : matrixFunction)  pair.second->Play();
	for (const auto& pair : physicsFunction) pair.second->Play();
	for (const auto& pair : lastFunction)	 pair.second->Play();


	while (RemoveList.empty() == false)
	{
		auto a = RemoveList.front();
		auto list = a->GetModules();
		for (auto K : list)
		{
			std::string key = std::to_string(a->GetHashCode()) + "_" + K->GetClassNameString();
			RemoveFunction(updateFunction, key);
			RemoveFunction(matrixFunction, key);
			RemoveFunction(physicsFunction, key);
			RemoveFunction(lastFunction, key);
		}
		RemoveList.pop();
	};
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

void FunctionManager::RegisterFunction(GameObject* obj, Module* module, int type)
{
	obj->GetHashCode();
	module->GetClassNameString();
	std::string key = std::to_string(obj->GetHashCode()) + "_" + module->GetClassNameString();
	auto a = Action::Create();
	
	//Awake에서 추가된 모듈들은 함수 호출이 가능하도록 
	if (isAwakeCall){a->isReady = true;}

	switch (type)
	{
	case AWAKE_FUNCTION :
		a->Setting(obj, module, key, [module] {module->Awake(); });
		awakeFunction.push(a);
		break;
	case START_FUNCTION:
		a->Setting(obj, module, key, [module] {module->Start(); });
		startFunction.push(a);
		break;
	case UPDATE_FUNCTION:
		a->Setting(obj, module, key, [module] {module->Update(); });
		updateFunction.insert({ key, a});
		break;
	case MATRIX_UPDATE_FUNCTION:
		a->Setting(obj, module, key, [module] {module->MatrixUpdate(); });
		matrixFunction.insert({ key, a });
		break;
	case PHYSICS_UPDATE_FUNCTION:
		a->Setting(obj, module, key, [module] {module->PhysicsUpdate(); });
		physicsFunction.insert({ key, a });
		break;
	case LAST_UPDATE_FUNCTION:
		a->Setting(obj, module, key, [module] {module->LastUpdate(); });
		lastFunction.insert({ key, a });
		break;
	}
}

void FunctionManager::RemoveFunction(GameObject* obj)
{
	RemoveList.push(obj);
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
