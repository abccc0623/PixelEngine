#include "pch.h"
#include "LuaStaticCreate.h"
#include <filesystem>
#include <fstream>


LuaStaticCreate::LuaStaticCreate()
{}

LuaStaticCreate::~LuaStaticCreate()
{}

void LuaStaticCreate::Generate(const std::string& outPath, std::map<std::string, PixelClassMeta>& staticClass)
{
	for (const auto& meta : staticClass)
	{
		std::string content;
		content += "local ffi = require(\"ffi\")\n";
		content += "local dll = ffi.load(\"PixelEngine\")\n\n";
		content += CreateClassAnnotation(meta.second);
		content += "ffi.cdef[[\n";
		content += CreateCDef(meta.second);
		content += "]]\n\n";
		content += "---@class " + meta.second.name + "\n";
		if (meta.second.name == "Entity")
		{
			content += CreateEntityScriptAnnotation();
		}
		content += meta.second.name + " = " + meta.second.name + " or {}\n\n";
		content += CreateFunction(meta.second);
		content += "return " + meta.second.name;
		std::filesystem::create_directories(outPath);
		std::ofstream file(outPath + "/" + meta.second.name + ".lua");
		file << content;
		file.close();
	}
}

std::string LuaStaticCreate::CreateFunction(const PixelClassMeta& PClass)
{
	std::string content;
	if (PClass.methods.size() > 0)
	{
		for (int i = 0; i < PClass.methods.size(); i++)
		{
			std::string returnType = PClass.methods[i].returnType;
			size_t pos = PClass.methods[i].name.find('_');
			std::string left = PClass.methods[i].name.substr(0, pos);
			std::string right = PClass.methods[i].name.substr(pos + 1);

			std::string propertys;
			for (int p = 0; p < PClass.methods[i].propertys.size(); p++)
			{
				propertys += "---@param " + PClass.methods[i].propertys[p].name + " " + ToLuaType(PClass.methods[i].propertys[p].type) + "\n";
			}

			if (returnType != "void")
			{
				propertys += "---@return " + ToLuaType(returnType) + "\n";
			}

			content += propertys;
			content += "function " + left + "." + right + "(" + CreateMethodParameter(PClass.methods[i]) + ")\n";
			if (returnType == "")
			{
				content += "\tdll." + PClass.methods[i].name + "(" + CreateMethodParameter(PClass.methods[i]) + ")\n";
			}
			else
			{
				content += "\treturn ";
				if (IsCStringType(returnType))
				{
					content += "ffi.string(";
				}
				content += "dll." + PClass.methods[i].name + "(" + CreateMethodParameter(PClass.methods[i]) + ")";
				if (IsCStringType(returnType))
				{
					content += ")";
				}
				content += "\n";
			}
			content += "end\n";
		}

	}

	return content;
}

std::string LuaStaticCreate::CreateEntityScriptAnnotation()
{
	std::string content;
	content += "---@field GetScript fun(ID: number): LuaInstance?\n";
	content += "---@field SetValue fun(ID: number, variableName: string, value: any): boolean\n";
	content += "---@field GetValue fun(ID: number, variableName: string): any\n";
	content += "---@field CallFunction fun(ID: number, functionName: string, ...: any): boolean, any\n";
	return content;
}
