#include "pch.h"
#include "MaterialFactory.h"
#include "PixelResources.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
void MaterialFactory::Initialize()
{

}

void MaterialFactory::Release()
{

}

void MaterialFactory::Clear()
{
	for (auto k : materialMap)
	{
		auto value = k.second;
		delete value;
		value = nullptr;
		k.second = nullptr;
	}
    materialMap.clear();

}

void* MaterialFactory::GetResource(std::string name)
{
    auto value = materialMap.find(name);
    if (value != materialMap.end())
    {
        return value->second;
    }
    return nullptr;
}

void* MaterialFactory::GetResource(Handle16 key)
{
    for (auto& K : materialMap)
    {
        if (K.second->key == key)
        {
            return K.second;
        }
    }
    return nullptr;
}

Handle16 MaterialFactory::SetResource(std::string name)
{
    std::filesystem::path p(name);
    std::string pureName = p.stem().string();

    std::string texturePath;
    float data[4];
    // 2. 파일 스트림 오픈
    std::ifstream fileStream(name);
    MaterialResources* m = new MaterialResources();
    if (fileStream.is_open())
    {
        std::string line;
        // 3. 한 줄씩 읽기
        while (std::getline(fileStream, line))
        {
            std::stringstream ss(line);
            std::string key, value;
            ss >> key >> value;

            if (key == "@Path") m->TexturePath = value.c_str();
            else if (key == "@OffsetX") m->Offset[0] = std::stof(value);
            else if (key == "@OffsetY") m->Offset[1] = std::stof(value);
            else if (key == "@TilingX") m->Tiling[0] = std::stof(value);
            else if (key == "@TilingY") m->Tiling[1] = std::stof(value);
        }
        fileStream.close();
    }
    m->key = keyAllocator.GetKey16();
    materialMap.insert({ pureName ,m });
    return m->key;
}

