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
}

FunctionManager::~FunctionManager()
{

}

void FunctionManager::Initialize()
{

}

void FunctionManager::Update()
{
	Action::StartReady();

	while (awakeFunction.empty() == false)
	{
		auto a = awakeFunction.front();
		a->Play();
		awakeFunction.pop();
		Action::Delete(a);
	};

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
}

void FunctionManager::Release()
{
	awakeFunction = {};
	startFunction = {};
	updateFunction.clear();
	matrixFunction.clear();
	physicsFunction.clear();
	lastFunction.clear();
}

void FunctionManager::RegisterFunction(GameObject* obj, Module* module, int type)
{
	obj->GetHashCode();
	module->GetClassNameString();
	std::string key = std::to_string(obj->GetHashCode()) + "_" + module->GetClassNameString();
	auto a = Action::Create();
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
