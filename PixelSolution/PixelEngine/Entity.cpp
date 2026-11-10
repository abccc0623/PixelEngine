#include "pch.h"
#include "Entity.h"
#include "PixelEngineAPI.h"
#include "EntityObject.h"
#include "PixelEngine.h"
#include "SceneManager.h"
#include "LuaManager.h"
#include "SceneObject.h"
#include "Transform.h"
#include "Camera.h"
#include "UIImage.h"

extern PixelEngine* Engine;
unsigned int Entity_Create(const char* scriptName)
{
	// Lua에서 nil을 넘겼을 때 std::string(nullptr)가 네이티브 예외를 일으키지 않게 막는다.
	if (scriptName == nullptr)
	{
		PixelLog::Error("[Entity][Create] scriptName is nil.");
		return UINT32_MAX;
	}
	std::string scriptNameSTR(scriptName);
	auto sceneManager = Engine->GetFactory<SceneManager>();
	auto luaManager = Engine->GetFactory<LuaManager>();
	auto id = sceneManager->CreateEntity(scriptNameSTR);
	return id;
}

unsigned int Entity_CreateEmpty()
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	return sceneManager->CreateEntity("");
}

unsigned int Entity_CreateCamera()
{
	const auto id = Entity_CreateEmpty();
	if (id == UINT32_MAX) return id;
	if (Transform_Add(id) == nullptr || Camera_Add(id) == nullptr)
	{
		Entity_Destroy(id);
		return UINT32_MAX;
	}
	return id;
}

UIImageData* Entity_Create2DImage(const char* textureName)
{
	if (textureName == nullptr)
	{
		PixelLog::Error("[Entity][Create2DImage] textureName is nil.");
		return nullptr;
	}
	const auto id = Entity_CreateEmpty();
	if (id == UINT32_MAX) return nullptr;
	if (Transform_Add(id) == nullptr)
	{
		Entity_Destroy(id);
		return nullptr;
	}
	auto image = UIImage_Add(id);
	if (image == nullptr)
	{
		Entity_Destroy(id);
		return nullptr;
	}
	UIImage_SetTexture(id, textureName);
	return image;
}

void Entity_Destroy(unsigned int id)
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	uint32_t u(id);
	sceneManager->DestroyEntity(u);
}

void Entity_SetActive(unsigned int id, bool active)
{
	auto find = FindEntity(id);
	if (find == nullptr)
	{
		PixelLog::Error("[Entity][SetActive] Invalid Entity ID: " + std::to_string(id));
		return;
	}
	find->SetActive(active);
}

bool Entity_GetActive(unsigned int id)
{
	auto find = FindEntity(id);
	if (find == nullptr)
	{
		PixelLog::Error("[Entity][GetActive] Invalid Entity ID: " + std::to_string(id));
		return false;
	}
	return find->GetActive();
}
