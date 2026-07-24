#include "pch.h"
#include "Group.h"
#include "PixelEngine.h"
#include "SceneObject.h"
#include "SceneManager.h"
#include "GroupManager.h"

extern PixelEngine* Engine;
unsigned int Group_CreateGroupAndEntity(const char* group, const char* scriptName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string groupNameSTR(group);
	std::string scriptNameSTR(scriptName);
	return scene->GetNowScene()->CreateGroupAndEntity(groupNameSTR, scriptNameSTR);
}

void Group_Set(const char* groupName, unsigned int id)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string groupNameSTR(groupName);
	scene->GetNowScene()->GetGroupManager()->Set(groupNameSTR, id);
}

void Group_Remove(const char* groupName, unsigned int id)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string groupNameSTR(groupName);
	scene->GetNowScene()->GetGroupManager()->Remove(groupNameSTR, id);
}

void Group_Clear(const char* groupName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string groupNameSTR(groupName);
	scene->GetNowScene()->GetGroupManager()->Clear(groupNameSTR);
}

int Group_Count(const char* groupName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string groupNameSTR(groupName);
	auto K = scene->GetNowScene()->GetGroupManager()->Get(groupNameSTR);
	return (int)K->size();
}

unsigned int Group_First(const char* groupName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string groupNameSTR(groupName);
	std::vector<unsigned int>* K = scene->GetNowScene()->GetGroupManager()->Get(groupNameSTR);
	if (K->size() != 0)
	{
		return (*K)[0];
	}
}

int Group_GetCount(const char* groupName)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string groupNameSTR(groupName);
	auto ids = scene->GetNowScene()->GetGroupManager()->Get(groupNameSTR);
	if (ids == nullptr)
	{
		return 0;
	}

	return static_cast<int>(ids->size());
}

unsigned int Group_GetID(const char* groupName, int index)
{
	SceneManager* scene = Engine->GetFactory<SceneManager>();
	std::string groupNameSTR(groupName);
	auto ids = scene->GetNowScene()->GetGroupManager()->Get(groupNameSTR);
	if (ids == nullptr || index < 0 || index >= static_cast<int>(ids->size()))
	{
		return 0;
	}

	return (*ids)[index];
}
