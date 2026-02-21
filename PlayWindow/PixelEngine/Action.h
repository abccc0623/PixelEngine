#pragma once
#include "PixelObject.h"
#include <functional>
#include <string>
#include <queue>
class GameObject;
class Module;
class Action : public PixelObject
{
public:
	Action();
	~Action();

	void Setting(GameObject* obj,std::string targetKey, std::function<void(void)> function);
	void Play();
	void Ready();
	void Clear();
	static Action* Create();
	static void Delete(Action* action);
	static void StartReady();
	static void Release();
	bool isReady = false;
private:
	std::function<void(void)> fun;
	std::string key;
	GameObject* target;
	Module* targetMoudle;

	static std::queue<Action*> IdleAcionQueue;
	static std::queue<Action*> RunAciontQueue;
};

