#include "pch.h"
#include "TextureManager.h"
#include "PixelGraphicsAPI.h"

TextureManager::TextureManager()
{

}
TextureManager::~TextureManager()
{
	
}

void TextureManager::Load(const std::string& path)
{
	ObjectID TextureID = LoadGraphicsTexture(path.c_str());
	std::filesystem::path p(path);
	std::string name = p.stem().string();
	TextureMap.insert({ name,TextureID });
}

ObjectID TextureManager::Get(const std::string& name)
{
	auto findData = TextureMap.find(name);
	if (findData != TextureMap.end())
	{
		return findData->second;
	}
	else 
	{
		std::cout << "Not Find Texture : " + name << std::endl;
		return ObjectID();
	}
}


void TextureManager::Initialize()
{

}

void TextureManager::Update()
{

}

void TextureManager::ReleaseShared()
{

}
