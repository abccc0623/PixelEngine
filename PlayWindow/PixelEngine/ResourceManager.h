#pragma once
#include "EngineManager.h"
#include <string>
enum RESOURCE_TYPE
{
	TEXTURE = 0,
};
using ObjectID = size_t;
class TextureManager;
class ResourceManager :public EngineManager
{
public:
	ResourceManager();
	~ResourceManager();

	void Initialize() override;
	void Update() override;
	void Release() override;

	void Load(RESOURCE_TYPE type,const std::string& filePath);
	ObjectID Get(RESOURCE_TYPE type, const std::string& name);
private:
	TextureManager* textureManager;
};

