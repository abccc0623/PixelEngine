#include "pch.h"
#include "LuaStaticCreate.h"
#include "PixelMetaAPI.h"
#include <filesystem>
#include <fstream>

LuaStaticCreate::LuaStaticCreate()
{
}

LuaStaticCreate::~LuaStaticCreate()
{
}

void LuaStaticCreate::Generate(const char* outPath, std::vector<PixelClassMeta>& types)
{
	for (auto& type : types)
	{
		if (ShouldGenerate(type))
		{
			GenerateLua(std::string(outPath), type);
		}
	}
}

void LuaStaticCreate::GenerateLua(const std::string& outPath, PixelClassMeta& meta)
{
	std::string className = meta.thisName;
	std::unordered_map<std::string, std::string> data;
	data["CLASS_NAME"] = className;
	data["CDEF"] = CreateCDef(meta, className);
	data["METHODS"] = CreateMethods(meta, className);

	const std::string luaTemplate =
		"local ffi = require(\"ffi\")\n"
		"local dll = ffi.load(\"PixelEngine\")\n\n"
		"ffi.cdef[[\n"
		"{{CDEF}}"
		"]]\n\n"
		"---@class {{CLASS_NAME}}\n"
		"{{CLASS_NAME}} = {{CLASS_NAME}} or {}\n\n"
		"{{METHODS}}"
		"return {{CLASS_NAME}}\n";

	std::string content = ApplyTemplate(luaTemplate, data);
	std::filesystem::create_directories(outPath);
	std::ofstream file(outPath + "/" + meta.thisName + ".lua");
	file << content;
	file.close();
}

bool LuaStaticCreate::ShouldGenerate(PixelClassMeta& meta)
{
	if (meta.metaType != META_TYPE::STATIC)
	{
		return false;
	}

	return meta.thisName == "Debug" ||
		meta.thisName == "Pool" ||
		meta.thisName == "Asset" ||
		meta.thisName == "Entity" ||
		meta.thisName == "Transform";
}

std::string LuaStaticCreate::CreateCDef(PixelClassMeta& meta, const std::string& className)
{
	std::string content;
	for (auto& method : meta.methods)
	{
		content += "\t" + ToCType(method.returnType) + " " + CreateNativeFunctionName(className, method.name) + "(";
		content += CreateArgumentList(method.propertys, true);
		content += ");\n";
	}
	return content;
}

std::string LuaStaticCreate::CreateMethods(PixelClassMeta& meta, const std::string& className)
{
	std::string content;
	for (auto& method : meta.methods)
	{
		std::string args = CreateArgumentList(method.propertys, false);
		std::string nativeName = CreateNativeFunctionName(className, method.name);
		std::string luaName = CreateLuaFunctionName(className, method.name);

		for (int propertyIndex = 0; propertyIndex < method.propertys.size(); propertyIndex++)
		{
			content += "---@param " + CreateArgumentName(method.propertys[propertyIndex], propertyIndex) + " " + ToLuaType(method.propertys[propertyIndex]) + "\n";
		}
		if (method.returnType != "void")
		{
			content += "---@return " + ToLuaType(method.returnType) + "\n";
		}

		content += "function " + className + "." + luaName + "(" + args + ")\n";
		if (method.returnType == "void")
		{
			content += "\tdll." + nativeName + "(" + args + ")\n";
		}
		else
		{
			content += "\treturn dll." + nativeName + "(" + args + ")\n";
		}
		content += "end\n\n";
	}
	return content;
}

