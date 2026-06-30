#include "pch.h"
#include "LuaClassCreate.h"
#include "PixelMetaAPI.h"
#include <filesystem>
#include <fstream>

LuaClassCreate::LuaClassCreate()
{
}

LuaClassCreate::~LuaClassCreate()
{
}

void LuaClassCreate::Generate(const char* outPath, std::vector<PixelClassMeta>& types)
{
	for (auto& type : types)
	{
		if (ShouldGenerate(type))
		{
			GenerateLua(std::string(outPath), type);
		}
	}
}

void LuaClassCreate::GenerateLua(const std::string& outPath, PixelClassMeta& meta)
{
	std::string className = meta.thisName;
	std::unordered_map<std::string, std::string> data;
	data["CLASS_NAME"] = className;
	data["CLASS_COMMENT"] = CreateClassComment(meta, className);
	data["CDEF"] = CreateCDef(meta, className);
	data["CONSTRUCTOR"] = CreateConstructor(meta, className);
	data["METHODS"] = CreateMethods(meta, className);
	data["ALIASES"] = CreateAliases(meta, className);

	const std::string luaTemplate =
		"local ffi = require(\"ffi\")\n"
		"local dll = ffi.load(\"PixelEngine\")\n"
		"{{CLASS_COMMENT}}"
		"ffi.cdef[[\n"
		"{{CDEF}}"
		"]]\n\n"
		"---@class {{CLASS_NAME}}\n"
		"{{CLASS_NAME}} = {{CLASS_NAME}} or {}\n\n"
		"{{CONSTRUCTOR}}"
		"{{METHODS}}"
		"{{ALIASES}}"
		"return {{CLASS_NAME}}\n";

	std::string content = ApplyTemplate(luaTemplate, data);
	std::filesystem::create_directories(outPath);
	std::ofstream file(outPath + "/" + meta.thisName + ".lua");
	file << content;
	file.close();
}

bool LuaClassCreate::ShouldGenerate(PixelClassMeta& meta)
{
	if (meta.metaType != META_TYPE::CLASS) return false;

	if (meta.thisName == "PVector3") return true;
	if (meta.thisName == "TransformData") return true;
	return false;
}

std::string LuaClassCreate::CreateClassComment(PixelClassMeta& meta, const std::string& className)
{
	if (!HasLuaMembers(meta))
	{
		return "";
	}

	std::string content = "\n---@class " + className + "\n";
	for (auto& member : meta.members)
	{
		if (!member.luaBind)
		{
			continue;
		}

		content += "---@field " + member.name + " " + ToLuaType(member.type) + "\n";
	}
	return content;
}

std::string LuaClassCreate::CreateCDef(PixelClassMeta& meta, const std::string& className)
{
	std::string content;
	if (HasLuaMembers(meta))
	{
		content += "\ttypedef struct {\n";
		for (auto& member : meta.members)
		{
			if (!member.luaBind)
			{
				continue;
			}

			content += "\t\t" + ToCType(member.type) + " " + member.name + ";\n";
		}
		content += "\t} " + className + ";" + "\n\n";
	}

	for (auto& method : meta.methods)
	{
		content += "\t" + ToCType(method.returnType) + " " + CreateNativeFunctionName(className, method.name) + "(";
		content += CreateArgumentList(method.propertys, true);
		content += ");\n";
	}
	return content;
}

std::string LuaClassCreate::CreateConstructor(PixelClassMeta& meta, const std::string& className)
{
	if (!HasLuaMembers(meta))
	{
		return "";
	}

	std::unordered_map<std::string, std::string> data;
	data["CLASS_NAME"] = className;
	data["STRUCT_NAME"] = className;
	data["ARGS"] = CreateFieldArgumentList(meta);
	data["FIELD_INITIALIZERS"] = CreateFieldInitializers(meta);

	const std::string constructorTemplate =
		"function {{CLASS_NAME}}.new({{ARGS}})\n"
		"\treturn ffi.new(\"{{STRUCT_NAME}}\", {\n"
		"{{FIELD_INITIALIZERS}}"
		"\t})\n"
		"end\n\n"
		"setmetatable({{CLASS_NAME}}, {\n"
		"\t__call = function(_, {{ARGS}})\n"
		"\t\treturn {{CLASS_NAME}}.new({{ARGS}})\n"
		"\tend\n"
		"})\n\n";

	return ApplyTemplate(constructorTemplate, data);
}

std::string LuaClassCreate::CreateMethods(PixelClassMeta& meta, const std::string& className)
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

std::string LuaClassCreate::CreateAliases(PixelClassMeta& meta, const std::string& className)
{
	//if (meta.thisName == "PVector3")
	//{
	//	return "Vector3 = Vector3 or " + className + "\n\n";
	//}
	return "";
}

std::string LuaClassCreate::CreateFieldArgumentList(PixelClassMeta& meta)
{
	std::string args;
	for (auto& member : meta.members)
	{
		if (!member.luaBind)
		{
			continue;
		}

		if (!args.empty())
		{
			args += ", ";
		}
		args += member.name;
	}
	return args;
}

std::string LuaClassCreate::CreateFieldInitializers(PixelClassMeta& meta)
{
	std::string content;
	for (auto& member : meta.members)
	{
		if (!member.luaBind)
		{
			continue;
		}

		content += "\t\t" + member.name + " = " + member.name + " or " + CreateDefaultValue(member.type) + ",\n";
	}
	return content;
}

bool LuaClassCreate::HasLuaMembers(PixelClassMeta& meta)
{
	for (auto& member : meta.members)
	{
		if (member.luaBind)
		{
			return true;
		}
	}
	return false;
}
