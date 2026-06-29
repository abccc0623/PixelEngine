#include "pch.h"
#include "LuaCreate.h"
#include "PixelMetaAPI.h"
#include <filesystem>
#include <fstream>
LuaCreate::LuaCreate()
{

}

LuaCreate::~LuaCreate()
{

}

void LuaCreate::Generate(const char* outPath, std::vector<PixelClassMeta>& types)
{
	for (auto& K : types)
	{
		if (K.thisName == "int")continue;
		if (K.thisName == "float")continue;
		if (K.thisName == "double")continue;
		if (K.thisName == "std::string")continue;
		if (K.thisName == "char")continue;
		if (K.thisName == "bool")continue;


		if (K.thisName == "Debug" ||
			K.thisName == "Pool" ||
			K.thisName == "Asset" ||
			K.thisName == "Entity")
		{
			GenerateLua(std::string(outPath), K);
		}
	}




}

void LuaCreate::GenerateLua(const std::string& outPath, PixelClassMeta& meta)
{
	std::string className = meta.thisName;
	int memberCount = meta.members.size();
	int methodsCount = meta.methods.size();

	std::string content = "";
	content += "local ffi = require(\"ffi\")\n";
	content += "local dll = ffi.load(\"PixelEngine\")\n";

	if (memberCount > 0)
	{
		content += "\n---@class " + className + "Data\n";
		for (int i = 0; i < memberCount; i++)
		{
			if (meta.members[i].luaBind == false)
			{
				continue;
			}

			content += "---@field " + meta.members[i].name + " " + ToLuaType(meta.members[i].type) + "\n";
		}
	}

	content += "ffi.cdef[[\n";
	if (memberCount > 0)
	{
		content += "\ttypedef struct {\n";
		for (int i = 0; i < memberCount; i++)
		{
			if (meta.members[i].luaBind == false)
			{
				continue;
			}

			content += "\t\t" + ToCType(meta.members[i].type) + " " + meta.members[i].name + ";\n";
		}
		content += "\t} " + className + "Data;\n\n";
	}

	for (int i = 0; i < methodsCount; i++)
	{
		content += "\t" + ToCType(meta.methods[i].returnType) + " " + CreateNativeFunctionName(className, meta.methods[i].name) + "(";
		content += CreateArgumentList(meta.methods[i].propertys, true);
		content += ");\n";
	}
	content += "]]\n\n";
	content += "---@class " + className + "\n";
	content += className + " = " + className + " or {}\n\n";

	for (int i = 0; i < methodsCount; i++)
	{
		PixelMethodMeta& method = meta.methods[i];
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

	content += "return " + className + "\n";
	std::filesystem::create_directories(outPath);
	std::ofstream file(outPath + "/" + meta.thisName + ".lua");
	file << content;
	file.close();
}

std::string LuaCreate::NormalizeType(const std::string& type)
{
	std::string normalized = type;
	std::erase(normalized, ' ');
	return normalized;
}

std::string LuaCreate::ToCType(const std::string& type)
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

std::string LuaCreate::ToLuaType(const std::string& type)
{
	std::string normalized = NormalizeType(type);
	if (normalized == "constchar*" || normalized == "charconst*" || normalized == "char*" || type == "std::string" || type == "string") return "string";
	if (type == "int" || type == "int32_t" || type == "unsigned int" || type == "uint32_t" || type == "float" || type == "double") return "number";
	if (type == "bool") return "boolean";
	if (type == "void") return "nil";
	return type;
}

std::string LuaCreate::CreateArgumentName(const std::string& type, int index)
{
	std::string name = ToLuaType(type);
	std::erase(name, '*');
	std::erase(name, '&');
	std::erase(name, ' ');
	std::erase(name, ':');
	if (name.empty() || name == "nil")
	{
		name = "value";
	}
	return name + std::to_string(index);
}

std::string LuaCreate::CreateArgumentList(const std::vector<std::string>& propertys, bool includeTypes)
{
	std::string args;
	for (int i = 0; i < propertys.size(); i++)
	{
		if (i != 0)
		{
			args += ", ";
		}

		if (includeTypes)
		{
			args += ToCType(propertys[i]) + " ";
		}
		args += CreateArgumentName(propertys[i], i);
	}
	return args;
}

std::string LuaCreate::CreateNativeFunctionName(const std::string& className, const std::string& methodName)
{
	std::string prefix = className + "_";
	if (methodName.rfind(prefix, 0) == 0)
	{
		return methodName;
	}
	return prefix + methodName;
}

std::string LuaCreate::CreateLuaFunctionName(const std::string& className, const std::string& methodName)
{
	std::string prefix = className + "_";
	if (methodName.rfind(prefix, 0) == 0)
	{
		return methodName.substr(prefix.size());
	}
	return methodName;
}
