#include "Action.h"
#include "Module.h"
#include "GameInput.h"


std::queue<Action*> Action::IdleAcionQueue = std::queue<Action*>();
std::queue<Action*> Action::RunAciontQueue = std::queue<Action*>();
Action::Action():
	fun(nullptr),
	target(nullptr),
	targetMoudle(nullptr),
	key(""),
	isReady(false)
{

}

Action::~Action()
{

}

void Action::Setting(GameObject* obj, Module* module,std::string targetKey, std::function<void(void)> function)
{
	target = obj;
	targetMoudle = module;
	fun = function;
	key = targetKey;
}

void Action::Play()
{
	if (isReady == true)
	{
		fun();
	}
}

void Action::Ready()
{
	isReady = true;
}

void Action::Clear()
{
	fun = nullptr;
	target = nullptr;
	targetMoudle = nullptr;
	key = "";
	isReady = false;
}

//새로운 함수포인트를 가져온다.
Action* Action::Create()
{
	if (IdleAcionQueue.empty())
	{
		for (int i = 0; i < 30; i++)
		{
			Action* a = new Action();
			IdleAcionQueue.push(a);
		}
	}
	auto target = IdleAcionQueue.front();
	target->Clear();
	IdleAcionQueue.pop();
	RunAciontQueue.push(target);
	return target;
}

//다 사용한 함수포인트를 다시 원래 상태로
void Action::Delete(Action* action)
{
	action->Clear();
	IdleAcionQueue.push(action);
}

//중간에 실행되지 않고 엔진이 업데이트 로직이 실행될때만 실행되도록 
void Action::StartReady()
{
	while(!RunAciontQueue.empty())
	{
		auto a = RunAciontQueue.front();
		a->isReady = true;
		RunAciontQueue.pop();
	}
}

