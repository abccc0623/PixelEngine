#pragma once
#include "EngineManager.h"
#include <map>
#include <functional>
#include <queue>
#include <typeinfo>
#include <concepts>
#include <type_traits>
#include "Core/Module.h"
#include <vector>


#define AWAKE_FUNCTION 0
#define START_FUNCTION 1
#define UPDATE_FUNCTION 2
#define MATRIX_UPDATE_FUNCTION 3
#define PHYSICS_UPDATE_FUNCTION 4
#define LAST_UPDATE_FUNCTION 5


template <typename T> class SPointer;
class GameObject;
class Module;
class CollisionManager;
class ObjectManager;
class FunctionManager : public EngineManager
{
public:
	FunctionManager();
	~FunctionManager();

	void Initialize() override;
	void Update() override;
	void ReleaseShared() override;

	//template <std::derived_from<Module> T>
	//void Register(GameObject* target);
	
	void FunctionUpdate();
	void Clear();
	void AddOneTimeFunction(SPointer<Module> module,int type);
	void AddTickFunction(SPointer<Module> module,int type);
private:
	std::map<int,std::queue <std::function<bool()>>> oneTime;
	std::map<int,std::vector <std::function<bool()>>> tickUpdate;

	std::map<int, std::queue <std::function<bool()>>> pendingTickUpdate;
	bool isRun = false;
	bool isClear = false;
};
