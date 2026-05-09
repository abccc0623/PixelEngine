#include "pch.h"
#include "PTransform.h"
#include "PixelEngine.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Registry.h"
extern PixelEngine* Engine;
void ECS::Transform::Add(unsigned int id)
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	auto scene = sceneManager->GetNowScene();
	auto registry = scene->GetRegistry();
	registry->AddComponent<TransformData>(id);
}
void ECS::Transform::SetPosition(unsigned int id, float x, float y, float z)
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	auto scene = sceneManager->GetNowScene();
	auto registry = scene->GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	data->position = {x ,y,z,0};
}

void ECS::Transform::SetRotation(unsigned int id, float x, float y, float z)
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	auto scene = sceneManager->GetNowScene();
	auto registry = scene->GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	data->rotation = { x ,y,z,0 };
}

void ECS::Transform::SetScale(unsigned int id, float x, float y, float z)
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	auto scene = sceneManager->GetNowScene();
	auto registry = scene->GetRegistry();
	TransformData* data = registry->Get<TransformData>(id);
	data->scale = { x ,y,z,0 };
}
