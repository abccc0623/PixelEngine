#include "pch.h"
#include "LuaEnumCreate.h"
#include <filesystem>
#include <fstream>

LuaEnumCreate::LuaEnumCreate()
{
}

LuaEnumCreate::~LuaEnumCreate()
{
}

void LuaEnumCreate::Generate(const std::string& outPath, std::map<std::string, PixelEnumMeta>& enumlist)
{
	std::string content;

	content += "local ffi = require(\"ffi\")\n\n";
	content += "ffi.cdef[[\n";
	for (auto& K : enumlist)
	{
		content += CreateCDef(K.second);
	}
	content += "]]\n\n";

	for (auto& K : enumlist)
	{
		content += "---@enum " + K.second.name + "\n";
		content += "local " + K.second.name + " = ";
		content += CreateEnumTable(K.second);
		content += "\n\n";
	}

	content += "return {\n";
	for (auto& K : enumlist)
	{
		content += "\t" + K.second.name + " = " + K.second.name + ",\n";
	}
	content += "}\n";

	std::filesystem::create_directories(outPath);
	std::ofstream file(outPath + "/Enum.lua");
	file << content;
	file.close();
}


void LuaEnumCreate::GenerateLua(const std::string& outPath, const PixelEnumMeta& meta)
{
	std::string content;

	content += "local ffi = require(\"ffi\")\n\n";
	content += "ffi.cdef[[\n";
	content += CreateCDef(meta);
	content += "]]\n\n";
	content += "---@enum " + meta.name + "\n";
	content += "local " + meta.name + " = ";
	content += CreateEnumTable(meta);
	content += "\n\nreturn " + meta.name + "\n";

	std::filesystem::create_directories(outPath);
	std::ofstream file(outPath + "/" + meta.name + ".lua");
	file << content;
	file.close();
}

std::string LuaEnumCreate::CreateCDef(const PixelEnumMeta& meta)
{
	return "\ttypedef int " + meta.name + ";\n";
}

std::string LuaEnumCreate::CreateEnumTable(const PixelEnumMeta& meta)
{
	std::string content = "{\n";
	for (int i = 0; i < meta.value.size(); i++)
	{
		content += "\t" + meta.value[i] + " = " + std::to_string(i) + ",\n";
	}
	content += "}";
	return content;
}
