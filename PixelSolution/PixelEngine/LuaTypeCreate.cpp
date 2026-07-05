#include "pch.h"
#include "LuaTypeCreate.h"
#include <filesystem>
#include <fstream>

LuaTypeCreate::LuaTypeCreate()
{
}

LuaTypeCreate::~LuaTypeCreate()
{
}

void LuaTypeCreate::Generate(const std::string& outPath, std::vector<PixelClassMeta>& classList)
{
	for (const auto& meta : classList)
	{
		if (!ShouldGenerate(meta))
		{
			continue;
		}

		GenerateLua(outPath, meta);
	}
}

void LuaTypeCreate::GenerateLua(const std::string& outPath, const PixelClassMeta& meta)
{
	std::string content;

	content += "local ffi = require(\"ffi\")\n\n";
	content += CreateClassComment(meta);
	if (HasLuaMembers(meta))
	{
		content += "ffi.cdef[[\n";
		content += CreateCDef(meta);
		content += "]]\n\n";
	}
	content += meta.name + " = " + meta.name + " or {}\n\n";
	content += CreateMetatype(meta);
	content += "return " + meta.name + "\n";

	std::filesystem::create_directories(outPath);
	std::ofstream file(outPath + "/" + meta.name + ".lua");
	file << content;
	file.close();
}

std::string LuaTypeCreate::CreateClassComment(const PixelClassMeta& meta)
{
	std::string content = "---@class " + meta.name + "\n";

	for (const auto& member : meta.members)
	{
		if (!member.luaBind)
		{
			continue;
		}

		content += "---@field " + member.name + " " + ToLuaType(member.type) + "\n";
	}

	return content + "\n";
}

std::string LuaTypeCreate::CreateCDef(const PixelClassMeta& meta)
{
	std::string content;
	content += "\ttypedef struct {\n";
	for (const auto& member : meta.members)
	{
		if (!member.luaBind)
		{
			continue;
		}

		content += "\t\t" + ToCType(member.type) + " " + member.name + ";\n";
	}
	content += "\t} " + meta.name + ";\n";
	return content;
}

std::string LuaTypeCreate::CreateMetatype(const PixelClassMeta& meta)
{
	if (!HasLuaMembers(meta))
	{
		return "";
	}

	return "ffi.metatype(\"" + meta.name + "\", { __index = " + meta.name + " })\n\n";
}

std::string LuaTypeCreate::NormalizeType(const std::string& type)
{
	std::string normalized = type;
	std::erase(normalized, ' ');
	return normalized;
}

std::string LuaTypeCreate::ToCType(const std::string& type)
{
	std::string normalized = NormalizeType(type);
	if (type == "void") return "void";
	if (normalized == "constchar*" || normalized == "charconst*" || normalized == "char*" || type == "std::string" || type == "string") return "const char*";
	if (type == "float") return "float";
	if (type == "double") return "double";
	if (type == "bool") return "bool";
	if (type == "int" || type == "int32_t") return "int";
	if (type == "unsigned int" || type == "uint32_t") return "uint32_t";
	return type;
}

std::string LuaTypeCreate::ToLuaType(const std::string& type)
{
	std::string normalized = NormalizeType(type);
	if (normalized == "constchar*" || normalized == "charconst*" || normalized == "char*" || type == "std::string" || type == "string") return "string";
	if (type == "int" || type == "int32_t" || type == "unsigned int" || type == "uint32_t" || type == "float" || type == "double") return "number";
	if (type == "bool") return "boolean";
	if (type == "void") return "nil";

	std::erase(normalized, '*');
	std::erase(normalized, '&');
	return normalized;
}

bool LuaTypeCreate::ShouldGenerate(const PixelClassMeta& meta)
{
	return meta.name == "PVector2" || meta.name == "PVector3";
}

bool LuaTypeCreate::HasLuaMembers(const PixelClassMeta& meta)
{
	for (const auto& member : meta.members)
	{
		if (member.luaBind)
		{
			return true;
		}
	}
	return false;
}
