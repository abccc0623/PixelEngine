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
		if (meta.second.name == "PVector3")
		{
			content += "---@operator add(PVector3): PVector3\n";
			content += "---@operator sub(PVector3): PVector3\n";
			content += "---@operator mul(number|PVector3): PVector3\n";
			content += "---@operator eq(PVector3): boolean\n";
			content += "---@operator lt(PVector3): boolean\n";
			content += "---@operator le(PVector3): boolean\n";
		}
		for (const auto& method : meta.second.methods)
		{
			if (method.name == meta.second.name + "_Create")
			{
				continue;
			}

			content += "---@field " + CreateLuaFunctionName(meta.second.name, method.name)
				+ " fun(self: " + meta.second.name;
			for (size_t i = 1; i < method.propertys.size(); ++i)
			{
				content += ", " + method.propertys[i].name + ": "
					+ ToLuaType(method.propertys[i].type);
			}
			content += ")";
			if (method.returnType != "void" && !method.returnType.empty())
			{
				content += ": " + ToLuaType(method.returnType);
			}
			content += "\n";
		}
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
		content += "\t\t" + CreateLuaFunctionName(PClass.name, PClass.methods[i].name)
			+ " = function(" + CreateMethodParameter(PClass.methods[i]) + ")\n";
		content += "\t\t\t" + CreateMethod(PClass.methods[i]);
		content += "\t\tend,\n";
	}
	content += "\t},\n";
	if (PClass.name == "PVector3")
	{
		content += CreateVector3Operator();
	}
	else if (PClass.name == "PVector2")
	{
		content += CreateVector2Operator();
	}
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

std::string LuaTypeCreate::CreateVector3Operator()
{
	std::string content;
	content += "\t__eq = function(a, b)\n";
	content += "\t\treturn a.x == b.x and a.y == b.y and a.z == b.z\n";
	content += "\tend,\n";

	content += "\t__lt = function(a, b)\n";
	content += "\t\treturn a.x < b.x and a.y < b.y and a.z < b.z\n";
	content += "\tend,\n";

	content += "\t__le = function(a, b)\n";
	content += "\t\treturn a.x <= b.x and a.y <= b.y and a.z <= b.z\n";
	content += "\tend,\n";

	content += "\t__add = function(a, b)\n";
	content += "\t\treturn dll.PVector3_Create(a.x + b.x, a.y + b.y, a.z + b.z)\n";
	content += "\tend,\n";

	content += "\t__sub = function(a, b)\n";
	content += "\t\treturn dll.PVector3_Create(a.x - b.x, a.y - b.y, a.z - b.z)\n";
	content += "\tend,\n";

	content += "\t__mul = function(a, b)\n";
	content += "\t\tif type(a) == \"number\" then\n";
	content += "\t\t\treturn dll.PVector3_Create(a * b.x, a * b.y, a * b.z)\n";
	content += "\t\tend\n";
	content += "\t\tif type(b) == \"number\" then\n";
	content += "\t\t\treturn dll.PVector3_Create(a.x * b, a.y * b, a.z * b)\n";
	content += "\t\tend\n";
	content += "\t\treturn dll.PVector3_Create(a.x * b.x, a.y * b.y, a.z * b.z)\n";
	content += "\tend\n";
	return content;
}

std::string LuaTypeCreate::CreateVector2Operator()
{
	std::string content;
	content += "\t__eq = function(a, b)\n";
	content += "\t\treturn a.x == b.x and a.y == b.y\n";
	content += "\tend,\n";

	content += "\t__lt = function(a, b)\n";
	content += "\t\treturn a.x < b.x and a.y < b.y\n";
	content += "\tend,\n";

	content += "\t__le = function(a, b)\n";
	content += "\t\treturn a.x <= b.x and a.y <= b.y\n";
	content += "\tend\n";
	return content;
}
