#include "pch.h"
#include "LuaStaticCreate.h"
#include <filesystem>
#include <fstream>

LuaStaticCreate::LuaStaticCreate()
{
}

LuaStaticCreate::~LuaStaticCreate()
{
}

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
		content += meta.second.name + " = " + meta.second.name + " or {}\n\n";
		content += CreateFunction(meta.second);
		content += "return " + meta.second.name;
		std::filesystem::create_directories(outPath);
		std::ofstream file(outPath + "/" + meta.second.name + ".lua");
		file << content;
		file.close();
	}
}

bool LuaStaticCreate::HasGeneratedMethods(const PixelStaticMeta& meta)
{
	for (const auto& method : meta.methods)
	{
		if (ShouldGenerateMethod(meta, method))
		{
			return true;
		}
	}
	return false;
}

/*
void LuaStaticCreate::GenerateLua(const std::string& outPath, const PixelStaticMeta& meta)
{
	//std::string content;
	//
	//content += "local ffi = require(\"ffi\")\n";
	//content += "local dll = ffi.load(\"PixelEngine\")\n\n";
	//content += "ffi.cdef[[\n";
	//content += CreateCDef(meta);
	//content += "]]\n\n";
	//content += "---@class " + meta.name + "\n";
	//content += meta.name + " = " + meta.name + " or {}\n\n";
	//content += CreateMethods(meta);
	//content += "return " + meta.name + "\n";
	//
	//std::filesystem::create_directories(outPath);
	//std::ofstream file(outPath + "/" + meta.name + ".lua");
	//file << content;
	//file.close();
}
*/


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
				content += "\treturn dll." + PClass.methods[i].name + "(" + CreateMethodParameter(PClass.methods[i]) + ")\n";
			}
			content += "end\n";
		}

	}

	return content;
}

std::string LuaStaticCreate::CreateMethods(const PixelStaticMeta& meta)
{
	std::string content;
	for (const auto& method : meta.methods)
	{
		if (!ShouldGenerateMethod(meta, method))
		{
			continue;
		}

		std::string args = CreateArgumentList(method.propertys, false);
		std::string nativeName = CreateNativeFunctionName(meta.name, method.name);
		std::string luaName = CreateLuaFunctionName(meta.name, method.name);

		if (!method.tooltip.empty())
		{
			content += "--- " + method.tooltip + "\n";
		}
		for (int propertyIndex = 0; propertyIndex < method.propertys.size(); propertyIndex++)
		{
			const PixelParameterMeta& parameter = method.propertys[propertyIndex];
			content += "---@param " + CreateArgumentName(parameter, propertyIndex) + " " + ToLuaType(parameter.type);
			if (!parameter.tooltip.empty())
			{
				content += " # " + parameter.tooltip;
			}
			content += "\n";
		}
		if (method.returnType != "void")
		{
			content += "---@return " + ToLuaType(method.returnType) + "\n";
		}

		content += "function " + meta.name + "." + luaName + "(" + args + ")\n";
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

std::string LuaStaticCreate::CreateArgumentList(const std::vector<PixelParameterMeta>& parameters, bool includeTypes)
{
	std::string args;
	for (int i = 0; i < parameters.size(); i++)
	{
		if (!args.empty())
		{
			args += ", ";
		}

		if (includeTypes)
		{
			args += ToCType(parameters[i].type) + " ";
		}
		args += CreateArgumentName(parameters[i], i);
	}
	return args;
}

std::string LuaStaticCreate::CreateArgumentName(const PixelParameterMeta& parameter, int index)
{
	if (!parameter.name.empty())
	{
		return parameter.name;
	}

	std::string name = ToLuaType(parameter.type);
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

std::string LuaStaticCreate::CreateNativeFunctionName(const std::string& className, const std::string& methodName)
{
	(void)className;
	return methodName;
}

std::string LuaStaticCreate::CreateLuaFunctionName(const std::string& className, const std::string& methodName)
{
	std::string prefix = className + "_";
	if (methodName.rfind(prefix, 0) == 0)
	{
		return methodName.substr(prefix.size());
	}
	return methodName;
}

std::string LuaStaticCreate::NormalizeType(const std::string& type)
{
	std::string normalized = type;
	std::erase(normalized, ' ');
	return normalized;
}

std::string LuaStaticCreate::ToCType(const std::string& type)
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

std::string LuaStaticCreate::ToLuaType(const std::string& type)
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

bool LuaStaticCreate::ShouldGenerateMethod(const PixelStaticMeta& meta, const PixelMethodMeta& method)
{
	if (!IsExportedNativeMethod(method.name) || !IsSupportedFFIType(method.returnType))
	{
		return false;
	}

	for (const auto& parameter : method.propertys)
	{
		if (!IsSupportedFFIType(parameter.type))
		{
			return false;
		}
	}

	if (meta.type != "Component")
	{
		return true;
	}

	return IsStaticComponentMethod(CreateLuaFunctionName(meta.name, method.name));
}

bool LuaStaticCreate::IsExportedNativeMethod(const std::string& methodName)
{
	if (methodName == "BackgroundColor" ||
		methodName == "ChangeScene" ||
		methodName == "CreateScene" ||
		methodName == "LoadScene" ||
		methodName == "GetKey" ||
		methodName == "GetKeyDown" ||
		methodName == "GetKeyUp" ||
		methodName == "GetMousePosition_X" ||
		methodName == "GetMousePosition_Y" ||
		methodName == "GetDeltaTime" ||
		methodName == "GetTotalTime" ||
		methodName == "GetFPS" ||
		methodName == "IsPlayMode")
	{
		return true;
	}

	return methodName.find('_') != std::string::npos;
}

bool LuaStaticCreate::IsSupportedFFIType(const std::string& type)
{
	std::string normalized = NormalizeType(type);
	return normalized.find("std::") == std::string::npos &&
		normalized.find("sol::") == std::string::npos &&
		normalized.find("class") == std::string::npos;
}

bool LuaStaticCreate::IsStaticComponentMethod(const std::string& luaName)
{
	return luaName.rfind("Get", 0) == 0 ||
		luaName.rfind("Add", 0) == 0 ||
		luaName.rfind("Has", 0) == 0;
}
