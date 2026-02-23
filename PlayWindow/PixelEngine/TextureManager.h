#pragma once

#include <string>
#include <unordered_map>
#include "EngineManager.h"
using ObjectID = size_t;
class TextureManager : public EngineManager
{
public:
	TextureManager();
	~TextureManager();

	void Initialize() override;
	void Update() override;
	void ReleaseShared() override;
public:
	void Load(const std::string& path);
	ObjectID Get(const std::string& name);
private:
	std::unordered_map<std::string,ObjectID> TextureMap;
};

