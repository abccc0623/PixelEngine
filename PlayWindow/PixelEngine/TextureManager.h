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
	void Release() override;
public:
	//void Initialize(GraphicEngineManager* Graphic, CRITICAL_SECTION* _cs);
	void Load(const std::string& path);
	ObjectID Get(const std::string& name);
public:
	//void BakeSkyLightMap(std::string& Path, bool hdri);
	//void BakeConvertSkyLightMap(std::string& Path, float angle, float threshold, bool hdri, UINT index);
	//void BakeConvertSkyCubeMap(std::string& Path, float angle, float threshold, bool hdri);
	
public:
	//void SaveConvertSkyLightMap(std::string& Path, std::string& SaveName);
	//void SaveConvertSkyCubeMap(std::string& Path, std::string& SaveName);
	//void SaveSpriteToVolumeTexture_LUT(std::string fileName, std::string saveName, UINT pixelSize);

private:
	//bool CheckFolder(std::string& Path);
	//void LoadFolder(std::string& Path);
	//void LoadFile(std::string& Path);
	//std::string CutFilePath(std::string& Path);
private:
	std::unordered_map<std::string,ObjectID> TextureMap;
};

