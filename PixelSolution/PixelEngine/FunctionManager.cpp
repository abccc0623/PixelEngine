#include "pch.h"
#include "FunctionManager.h"
#include "Core/GameObject.h"
#include "Core/PixelObject.h"
#include "PixelEngine.h"
#include "Core/Module.h"
#include "SPointer.h"
#include "WPointer.h"

FunctionManager::FunctionManager()
{
	
	oneTime.insert({ AWAKE_FUNCTION , std::queue<std::function<bool()>>() });
	oneTime.insert({ START_FUNCTION , std::queue<std::function<bool()>>() });
	tickUpdate.insert({ UPDATE_FUNCTION ,std::vector <std::function<bool()>>() });
	tickUpdate.insert({ MATRIX_UPDATE_FUNCTION ,std::vector <std::function<bool()>>() });
	tickUpdate.insert({ PHYSICS_UPDATE_FUNCTION ,std::vector <std::function<bool()>>() });
	tickUpdate.insert({ LAST_UPDATE_FUNCTION ,std::vector <std::function<bool()>>() });

	//지연처리를 위한 데이터
	pendingTickUpdate.insert({ AWAKE_FUNCTION , std::queue<std::function<bool()>>() });
	pendingTickUpdate.insert({ START_FUNCTION , std::queue<std::function<bool()>>() });
	pendingTickUpdate.insert({ UPDATE_FUNCTION , std::queue<std::function<bool()>>() });
	pendingTickUpdate.insert({ MATRIX_UPDATE_FUNCTION , std::queue<std::function<bool()>>() });
	pendingTickUpdate.insert({ PHYSICS_UPDATE_FUNCTION , std::queue<std::function<bool()>>() });
	pendingTickUpdate.insert({ LAST_UPDATE_FUNCTION , std::queue<std::function<bool()>>() });
}

FunctionManager::~FunctionManager()
{

}

void FunctionManager::Initialize(){}
void FunctionManager::Update(){}
void FunctionManager::ReleaseShared(){}

void FunctionManager::FunctionUpdate()
{
	//지연처리
	for (auto& [priority, funcQueue] : pendingTickUpdate)
	{
		while (!funcQueue.empty())
		{
			auto func = std::move(funcQueue.front()); // std::move로 성능 최적화
			funcQueue.pop();

			if (priority == AWAKE_FUNCTION || priority == START_FUNCTION)
			{
				oneTime[priority].push(func);
			}
			else 
			{
				tickUpdate[priority].push_back(func);
			}
		}
	}

	//실제 업데이트
	isRun = true;
	for (auto& [priority, funcQueue] : oneTime)
	{
		while (!funcQueue.empty())
		{
			funcQueue.front()();
			funcQueue.pop();
		}
	}
	for (auto& [priority, funcVector] : tickUpdate)
	{
		std::erase_if(funcVector, [](auto& func) 
			{
				return !func();
			});
	}
	isRun = false;



	if (isClear == true)
	{
		oneTime.clear();
		tickUpdate.clear();
		isClear = false;
	}
}

void FunctionManager::Clear()
{
	isClear = true;
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
	
	if (isRun == false)
	{
		oneTime[type].push(OneTimeFunction);
	}
	else 
	{
		pendingTickUpdate[type].push(OneTimeFunction);
	}
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
	
	if (isRun == false)
	{
		tickUpdate[type].push_back(TickFunction);
	}
	else
	{
		pendingTickUpdate[type].push(TickFunction);
	}
}

