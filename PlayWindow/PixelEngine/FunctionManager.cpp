#include "FunctionManager.h"
#include "GameObject.h"

FunctionManager::FunctionManager()
{
	awakeFunction   = std::queue<std::function<void(void)>>();
	startFunction   = std::queue<std::function<void(void)>>();
	updateFunction  = std::unordered_map<std::string, std::function<void(void)>>();
	matrixFunction  = std::unordered_map<std::string, std::function<void(void)>>();
	physicsFunction = std::unordered_map<std::string, std::function<void(void)>>();
	lastFunction	= std::unordered_map<std::string, std::function<void(void)>>();
}

FunctionManager::~FunctionManager()
{

}

void FunctionManager::Initialize()
{

}

void FunctionManager::Update()
{
	while (awakeFunction.empty() == false)
	{
		std::function<void(void)> func =awakeFunction.front();
		func();
		awakeFunction.pop();
	};

	while (startFunction.empty() == false)
	{
		std::function<void(void)> func = startFunction.front();
		func();
		startFunction.pop();
	};

	for (const auto& pair : updateFunction)  pair.second();
	for (const auto& pair : matrixFunction)  pair.second();
	for (const auto& pair : physicsFunction) pair.second();
	for (const auto& pair : lastFunction)	 pair.second();
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
	switch (type)
	{
	case AWAKE_FUNCTION :
		awakeFunction.push([module] {module->Awake(); });
		break;
	case START_FUNCTION:
		startFunction.push([module] {module->Start(); });
		break;
	case UPDATE_FUNCTION:
		updateFunction.insert({ key, [module] {module->Update();}});
		break;
	case MATRIX_UPDATE_FUNCTION:
		matrixFunction.insert({ key, [module] {module->MatrixUpdate(); } });
		break;
	case PHYSICS_UPDATE_FUNCTION:
		physicsFunction.insert({ key, [module] {module->PhysicsUpdate(); } });
		break;
	case LAST_UPDATE_FUNCTION:
		lastFunction.insert({ key, [module] {module->LastUpdate();}});
		break;
	}
}
