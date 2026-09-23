#include "pch.h"
#include "Event.h"
#include "Registry.h"
#include "PixelEngine.h"
#include "EventManager.h"
#include "SceneObject.h"
#include "SceneManager.h"
extern PixelEngine* Engine;
bool Event_Bind(unsigned int id, const char* EventName, const char* bindFunctionName)
{
	if (EventName == nullptr || bindFunctionName == nullptr || EventName[0] == '\0' || bindFunctionName[0] == '\0') return false;

	auto sceneManager = Engine->GetFactory<SceneManager>();
	if (sceneManager == nullptr) return false;

	auto scene = sceneManager->GetNowScene();
	if (scene == nullptr || scene->GetEventManager() == nullptr) return false;

	return scene->GetEventManager()->BindLuaEvent(id, EventName, bindFunctionName);
}

bool Event_Call(const char* EventName)
{
	if (EventName == nullptr || EventName[0] == '\0') return false;

	auto sceneManager = Engine->GetFactory<SceneManager>();
	if (sceneManager == nullptr) return false;

	auto scene = sceneManager->GetNowScene();
	if (scene == nullptr || scene->GetEventManager() == nullptr) return false;

	return scene->GetEventManager()->CallLuaEvent(EventName);
}
