#include "pch.h"
#include "ResourceManager.h"
#include "TextureManager.h"
ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::Initialize()
{
	textureManager = new TextureManager();
}

void ResourceManager::Update()
{

}

void ResourceManager::ReleaseShared()
{
	delete textureManager;
	textureManager = nullptr;
}

void ResourceManager::Load(RESOURCE_TYPE type,const std::string& filePath)
{
	switch (type)
	{
	case RESOURCE_TYPE::TEXTURE:
		textureManager->Load(filePath);
		break;
	}
}

ObjectID ResourceManager::Get(RESOURCE_TYPE type, const std::string& name)
{
	switch (type)
	{
		case RESOURCE_TYPE::TEXTURE:
			return textureManager->Get(name);
	}
	return ObjectID();
}
