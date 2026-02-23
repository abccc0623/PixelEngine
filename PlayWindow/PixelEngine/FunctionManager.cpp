#include "FunctionManager.h"
#include "GameObject.h"
#include <iostream>
#include <algorithm>
#include "PixelObject.h"
#include "PixelEngine.h"
#include "Module.h"
#include "CollisionManager.h"
#include "ObjectManager.h"

FunctionManager::FunctionManager()
{
	oneTime.insert({ AWAKE_FUNCTION , std::queue<std::function<bool()>>() });
	oneTime.insert({ START_FUNCTION , std::queue<std::function<bool()>>() });
	tickUpdate.insert({ UPDATE_FUNCTION ,std::vector <std::function<bool()>>() });
	tickUpdate.insert({ MATRIX_UPDATE_FUNCTION ,std::vector <std::function<bool()>>() });
	tickUpdate.insert({ PHYSICS_UPDATE_FUNCTION ,std::vector <std::function<bool()>>() });
	tickUpdate.insert({ LAST_UPDATE_FUNCTION ,std::vector <std::function<bool()>>() });
}

FunctionManager::~FunctionManager()
{

}

void FunctionManager::Initialize(){}
void FunctionManager::Update(){}
void FunctionManager::ReleaseShared()
{
	oneTime.clear();
	tickUpdate.clear();
}

void FunctionManager::FunctionUpdate()
{
	for (auto& [priority, funcQueue] : oneTime)
	{
		if (priority == AWAKE_FUNCTION){isAwakeRun = true;}
		if (priority == START_FUNCTION){isStartRun = true;}

		while (!funcQueue.empty())
		{
			funcQueue.front()();
			funcQueue.pop();
		}
		isAwakeRun = false;
		isStartRun = false;
	}

	isUpdateRun = true;
	for (auto& [priority, funcVector] : tickUpdate)
	{
		std::remove_if(funcVector.begin(), funcVector.end(),
			[](std::function<bool()>& func)
			{
				return !func();
			}),
			funcVector.end();
	}
	isUpdateRun = false;
}

void FunctionManager::AddOneTimeFunction(SPointer<Module> module, int type)
{
	WPointer<Module> w = module;
	
	//한번만 호출되는애들 전용
	auto OneTimeFunction = [w, type]() mutable
		{
			if (w.IsValid())
			{
				auto func = w.Lock();
				switch (type)
				{
				case AWAKE_FUNCTION:
					func->Awake();
					break;
				case START_FUNCTION:
					func->Start();
					break;
				}
			}
			return false;
		};
	
	oneTime[type].push(OneTimeFunction);
}

void FunctionManager::AddTickFunction(SPointer<Module> module, int type)
{
	WPointer<Module> w = module;
	
	//매프래임 호출되는 애들 전용
	auto TickFunction = [w,type]() mutable
		{
			if (w.IsValid())
			{
				auto func = w.Lock();
				switch (type)
				{
				case UPDATE_FUNCTION:
					func->Update();
					break;
				case MATRIX_UPDATE_FUNCTION:
					func->MatrixUpdate();
					break;
				case PHYSICS_UPDATE_FUNCTION:
					func->PhysicsUpdate();
					break;
				case LAST_UPDATE_FUNCTION:
					func->LastUpdate();
					break;
				}
				return true;
			}
			return false;
		};
	tickUpdate[type].push_back(TickFunction);
}

