#include "pch.h"
#include "ResourceManager.h"
#include "TextureManager.h"
#include "MaterialManager.h"
ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::Initialize()
{
	textureManager = new TextureManager();
	materialManager = new MaterialManager();
}

void ResourceManager::Update()
{

}

void ResourceManager::Release()
{
	delete textureManager;
	textureManager = nullptr;

	delete materialManager;
	materialManager = nullptr;
}

void ResourceManager::Load(RESOURCE_TYPE type,const std::string& filePath)
{
	switch (type)
	{
	case RESOURCE_TYPE::TEXTURE:
		textureManager->Load(filePath);
		break;
	case RESOURCE_TYPE::MATERIAL:
		materialManager->Load(filePath);
		break;
	}
}

ObjectID ResourceManager::Get(RESOURCE_TYPE type, const std::string& name)
{
	switch (type)
	{
		case RESOURCE_TYPE::TEXTURE:
			return textureManager->Get(name);
		case RESOURCE_TYPE::MATERIAL:
			return materialManager->Get(name);
	}
	return ObjectID();
}

void ResourceManager::Clear()
{
	textureManager->Clear();
	materialManager->Clear();
}
