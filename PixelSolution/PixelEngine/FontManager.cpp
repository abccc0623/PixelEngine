#include "pch.h"
#include "FontManager.h"
#include "PixelGraphicsAPI.h"
#include "Log.h"

FontManager::FontManager() = default;
FontManager::~FontManager() = default;

void FontManager::Load(const std::string& path)
{
	const ObjectID fontID = LoadGraphicsFont(path.c_str());
	const std::filesystem::path fontPath(path);
	FontMap.insert_or_assign(fontPath.stem().string(), fontID);
}

ObjectID FontManager::Get(const std::string& name)
{
	const auto found = FontMap.find(name);
	if (found != FontMap.end())
	{
		return found->second;
	}

	PixelLog::Error("Not Find Font : " + name);
	return ObjectID();
}

void FontManager::Initialize() {}
void FontManager::Update() {}
void FontManager::Release() { FontMap.clear(); }
void FontManager::Clear() { FontMap.clear(); }
