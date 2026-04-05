#include "pch.h"
#include "MaterialManager.h"
#include "PixelGraphicsAPI.h"

MaterialManager::MaterialManager()
{

}

MaterialManager::~MaterialManager()
{

}

void MaterialManager::Initialize()
{

}

void MaterialManager::Update()
{

}

void MaterialManager::Release()
{
	MaterialMap.clear();
}

void MaterialManager::Clear()
{
	MaterialMap.clear();
}

ObjectID MaterialManager::Load(const std::string& path)
{
	ObjectID MaterialID = LoadGraphicsMaterial(path.c_str());
	std::filesystem::path p(path);
	std::string name = p.stem().string();
	MaterialMap.insert({ name,MaterialID });
	return MaterialID;
}

ObjectID MaterialManager::Get(const std::string& name)
{
	auto findData = MaterialMap.find(name);
	if (findData != MaterialMap.end())
	{
		return findData->second;
	}
	else
	{
		PixelLog::Error("Not Find Material : " + name);
		return ObjectID();
	}
}
