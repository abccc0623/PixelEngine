#include "pch.h"
#include "Asset.h"
#include "PixelEngine.h"
#include "LuaManager.h"
#include "ResourceManager.h"
#include "JsonManager.h"

extern PixelEngine* Engine;
static void ImportFile(const std::filesystem::path& path)
{
	std::string targetPath = path.generic_string();
	std::string fileName = path.stem().string();
	std::string ext = path.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char character)
		{
			return static_cast<char>(std::tolower(character));
		});

	if (ext == ".lua" || ext == ".scene" || ext == ".pxm")
	{
		auto lua = Engine->GetFactory<LuaManager>();
		lua->ImportLua(targetPath, fileName, ext);
	}
	else if (ext == ".png" || ext == ".jpg")
	{
		auto resource = Engine->GetFactory<ResourceManager>();
		resource->Load(TEXTURE, targetPath);
	}
	else if (ext == ".mat")
	{
		auto resource = Engine->GetFactory<ResourceManager>();
		resource->Load(MATERIAL, targetPath);
	}
	else if (ext == ".json")
	{
		auto jsonFile = Engine->GetFactory<JsonManager>();
		jsonFile->Load(targetPath);
	}
}


void Asset_Import(const char* path)
{
	if (Engine == nullptr || path == nullptr || path[0] == '\0')
	{
		return;
	}

	std::filesystem::path targetPath(path);
	std::error_code error;
	std::filesystem::file_status status = std::filesystem::status(targetPath, error);
	if (error || !std::filesystem::exists(status))
	{
		PixelLog::Error("Path does not exist: " + targetPath.generic_string());
		return;
	}

	if (std::filesystem::is_regular_file(status))
	{
		ImportFile(targetPath);
		return;
	}

	if (!std::filesystem::is_directory(status))
	{
		return;
	}

	std::filesystem::recursive_directory_iterator iterator(
		targetPath,
		std::filesystem::directory_options::skip_permission_denied,
		error);
	std::filesystem::recursive_directory_iterator end;
	while (iterator != end)
	{
		if (error)
		{
			PixelLog::Warn("Failed to scan asset directory: " + error.message());
			error.clear();
			iterator.increment(error);
			continue;
		}

		if (iterator->is_regular_file(error) && !error)
		{
			ImportFile(iterator->path());
		}
		else
		{
			error.clear();
		}

		iterator.increment(error);
	}
}
