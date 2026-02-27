#pragma once
#include <windows.h>
#include "ResourceFactory.h"
#include "PixelResources.h"
struct TextureResources;
class TextureFactory :public ResourceFactory
{
public:
	TextureFactory();
	~TextureFactory();

	// ResourceFactory을(를) 통해 상속됨
	void Initialize() override;
	void Release() override;
	void* GetResource(std::string name) override;
	void* GetResource(Handle16 key) override;
	Handle16 SetResource(std::string name);
	TextureResources* CreateTextureResource(const char* filePath);
	TextureResources* DefaultTextureResource(const wchar_t* resourceName);
private:
	std::unordered_map<Handle16, TextureResources*> textureMap;
	std::unordered_map<std::string, TextureResources*> defaulttextureMap;
};

