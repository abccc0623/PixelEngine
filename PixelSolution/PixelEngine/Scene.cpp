#include "pch.h"
#include "Scene.h"
#include "PixelEngine.h"
#include "SceneManager.h"
extern PixelEngine* Engine;
void Scene_Change(const char* sceneName)
{
	std::string strPath(sceneName);
	if (Engine != nullptr)
	{
		SceneManager* scene = Engine->GetFactory<SceneManager>();
		scene->ChangeScene(strPath);
	}
}
