#include "pch.h"
#include "Group.h"
#include "PixelEngine.h"
#include "Scene.h"
#include "SceneManager.h"
#include "GroupManager.h"

extern PixelEngine* Engine;
unsigned int ECS::Group::CreateEntity(const char* groupName, const char* scriptName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string groupNameSTR(groupName);
	std::string scriptNameSTR(scriptName);

	return scene->GetNowScene()->CreateGroupEntity(groupNameSTR, scriptNameSTR);
}

void ECS::Group::Set(const char* groupName, unsigned int id)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	scene->GetNowScene()->GetGroupManager()->Set(groupName, id);
}

void ECS::Group::Remove(const char* groupName, unsigned int id)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	scene->GetNowScene()->GetGroupManager()->Remove(groupName, id);
}

void ECS::Group::Clear(const char* groupName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	scene->GetNowScene()->GetGroupManager()->Clear(groupName);
}

sol::as_table_t<std::vector<unsigned int>> ECS::Group::Get(const char* groupName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	auto ids = scene->GetNowScene()->GetGroupManager()->Get(groupName);
	if (ids == nullptr)
	{
		return sol::as_table(std::vector<unsigned int>());
	}

	return sol::as_table(*ids);
}
