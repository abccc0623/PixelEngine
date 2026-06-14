#include "pch.h"
#include "JsonManager.h"
#include "json.hpp"
#include <fstream>
#include "PixelEngineAPI.h"
#include "Export/Core/Module.h"
#include "PixelObjectLayerPairFilter.h"


JsonManager::JsonManager()
{

}

JsonManager::~JsonManager()
{

}

bool JsonManager::Load(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open()) return false;

	try
	{
		nlohmann::ordered_json LoadFile = nlohmann::ordered_json::parse(file);
		const std::string fileType = LoadFile.value("fileType", LoadFile.value("FileType", ""));

		if (fileType == "SceneFile")
		{
			LoadScene(LoadFile);
			return true;
		}
		else if (fileType == "LayerMatrix")
		{
			LoadLayer(LoadFile);
			return true;
		}
		PixelLog::Warn("Unsupported JSON file type: " + fileType);
		return false;
	}
	catch (const nlohmann::json::exception& error)
	{
		PixelLog::Error("Failed to load JSON: " + std::string(error.what()));
		return false;
	}
}
bool JsonManager::Save(const std::string& path, const nlohmann::ordered_json& j)
{
	std::ofstream file(path);
	if (!file.is_open()) return false;
	file << j.dump(4);
	return true;
}

void JsonManager::Initialize()
{


}

void JsonManager::Update()
{
}

void JsonManager::Release()
{

}

void JsonManager::Clear()
{

}

void JsonManager::LoadLayer(nlohmann::ordered_json file)
{
	if (!file.contains("layers") || !file["layers"].is_array() ||
		!file.contains("collisionMatrix") || !file["collisionMatrix"].is_array())
	{
		PixelLog::Error("LayerMatrix JSON requires layers and collisionMatrix arrays.");
		return;
	}

	const auto& layers = file["layers"];
	const auto& matrix = file["collisionMatrix"];
	const size_t layerCount = layers.size();

	if (layerCount == 0 || layerCount > Layers::NUM_LAYERS || matrix.size() != layerCount)
	{
		PixelLog::Error("LayerMatrix JSON has an invalid layer count or matrix size.");
		return;
	}

	std::vector<std::string> loadedNames;
	std::vector<std::vector<bool>> loadedMatrix(layerCount, std::vector<bool>(layerCount));
	loadedNames.reserve(layerCount);

	for (size_t row = 0; row < layerCount; ++row)
	{
		if (!matrix[row].is_array() || matrix[row].size() != layerCount)
		{
			PixelLog::Error("LayerMatrix collisionMatrix must be a square matrix.");
			return;
		}

		loadedNames.push_back(layers[row].get<std::string>());
		for (size_t col = 0; col < layerCount; ++col)
		{
			loadedMatrix[row][col] = matrix[row][col].get<bool>();
		}
	}

	for (JPH::uint32 row = 0; row < Layers::NUM_LAYERS; ++row)
	{
		for (JPH::uint32 col = 0; col < Layers::NUM_LAYERS; ++col)
		{
			Layers::collisionMatrix[row][col] = false;
		}
	}

	for (size_t row = 0; row < layerCount; ++row)
	{
		for (size_t col = 0; col < layerCount; ++col)
		{
			Layers::collisionMatrix[row][col] = loadedMatrix[row][col];
		}
	}
	Layers::layerNames = loadedNames;
	Layers::layerCount = static_cast<JPH::uint32>(layerCount);
}

void JsonManager::LoadScene(nlohmann::ordered_json file)
{
}

void JsonManager::LoadGameObject(nlohmann::ordered_json file)
{
}
