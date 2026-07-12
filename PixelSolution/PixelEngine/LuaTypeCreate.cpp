#include "pch.h"
#include "LuaTypeCreate.h"
#include <filesystem>
#include <fstream>

LuaTypeCreate::LuaTypeCreate()
{}

LuaTypeCreate::~LuaTypeCreate()
{}

void LuaTypeCreate::Generate(const std::string& outPath, std::map<std::string, PixelClassMeta>& classList)
{
	for (const auto& meta : classList)
	{
		std::string content;
		content += "local ffi = require(\"ffi\")\n";
		content += "local dll = ffi.load(\"PixelEngine\")\n\n";
		content += CreateClassAnnotation(meta.second);
		content += "ffi.cdef[[\n";
		content += CreateCDef(meta.second);
		content += "]]\n\n";
		content += CreateFunction(meta.second);
		content += "return " + meta.second.name;
		std::filesystem::create_directories(outPath);
		std::ofstream file(outPath + "/" + meta.second.name + ".lua");
		file << content;
		file.close();
	}
}

std::string LuaTypeCreate::CreateFunction(const PixelClassMeta& PClass)
{
	std::string content;

	auto it = std::find_if(
		PClass.methods.begin(),
		PClass.methods.end(),
		[&PClass](const PixelMethodMeta& method)
		{
			return method.name == PClass.name + "_Create";
		}
	);

	content += "local " + PClass.name + "_value_mt = {\n";
	content += "\t__index = {\n";
	for (int i = 0; i < PClass.methods.size(); i++)
	{
		if (PClass.methods[i].name.find("_Create") != std::string::npos)
		{
			continue;
		}
		content += "\t\t" + PClass.methods[i].name + " = function(" + CreateMethodParameter(PClass.methods[i]) + ")\n";
		content += "\t\t\t" + CreateMethod(PClass.methods[i]);
		content += "\t\tend,\n";
	}
	content += "\t}\n";
	content += "}\n\n";
	content += "ffi.metatype(\"" + PClass.name + "\", " + PClass.name + "_value_mt)\n\n";
	content += "---@class " + PClass.name + "\n";
	content += PClass.name + " = " + PClass.name + " or {}\n";

	if (it != PClass.methods.end())
	{
		std::string p = CreateMethodParameter(*it);
		content += "setmetatable(" + PClass.name + ", {\n";
		content += "\t__call = function(_," + p + ")\n";
		content += "\t\treturn dll." + (*it).name + "(" + p + ")\n";
		content += "\tend\n";
		content += "})\n";
	}
	content += "\n";

	return content;
}
