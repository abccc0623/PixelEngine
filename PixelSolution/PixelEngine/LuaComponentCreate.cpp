#include "pch.h"
#include "LuaComponentCreate.h"
#include "PixelMetaAPI.h"
#include <filesystem>
#include <fstream>

LuaComponentCreate::LuaComponentCreate()
{
}

LuaComponentCreate::~LuaComponentCreate()
{
}

void LuaComponentCreate::Generate(const std::string& outPath, std::map<std::string, PixelClassMeta>& component)
{
	for (const auto& meta : component)
	{
		std::string content;
		content += "local ffi = require(\"ffi\")\n";
		content += "local dll = ffi.load(\"PixelEngine\")\n\n";
		content += CreateClassAnnotation(meta.second);
		content += "ffi.cdef[[\n";
		content += CreateCDef(meta.second);
		content += "]]\n\n";
		content += CreateFunction(meta.second);
		content += "ffi.metatype(\"" + meta.second.name + "Data\", " + meta.second.name + "_mt)\n\n";
		content += meta.second.name + " = " + meta.second.name + " or {}\n\n";
		content += CreateBaseFunction(meta.second);

		content += "return " + meta.second.name;
		std::filesystem::create_directories(outPath);
		std::ofstream file(outPath + "/" + meta.second.name + ".lua");
		file << content;
		file.close();
	}
}

std::string LuaComponentCreate::CreateBaseFunction(const PixelClassMeta& PClass)
{
	std::string content;

	content += "---@param ID number \n";
	content += "---@return {{CLASS_NAME}}Data \n";
	content += "function {{CLASS_NAME}}.Add(ID) \n";
	content += "\treturn dll.{{CLASS_NAME}}_Add(ID) \n";
	content += "end \n\n";

	content += "---@param ID number \n";
	content += "---@return {{CLASS_NAME}}Data \n";
	content += "function {{CLASS_NAME}}.Get(ID) \n";
	content += "\treturn dll.{{CLASS_NAME}}_Get(ID) \n";
	content += "end \n\n";

	content += "---@param ID number \n";
	content += "---@return boolean \n";
	content += "function {{CLASS_NAME}}.Has(ID) \n";
	content += "\treturn dll.{{CLASS_NAME}}_Has(ID) \n";
	content += "end \n\n";

	return ReplaceAll(content, "CLASS_NAME", PClass.name);
}

std::string LuaComponentCreate::CreateFunction(const PixelClassMeta& PClass)
{
	std::string content;

	content += "local " + PClass.name + "_mt = {\n";
	content += "\t__index = {\n";

	for (int i = 0; i < PClass.methods.size(); i++)
	{
		if (PClass.methods[i].name == PClass.name + "_Add" ||
			PClass.methods[i].name == PClass.name + "_Get" ||
			PClass.methods[i].name == PClass.name + "_Has")
		{
			continue;
		}

		content += "\t\t" + PClass.methods[i].name + " = function(" + CreateMethodParameter(PClass.methods[i]) + ")\n";
		content += "\t\t\t" + CreateMethod(PClass.methods[i]);
		content += "\t\tend\n";
	}

	content += "\t}\n";
	content += "}\n\n";
	return content;
}



std::string LuaComponentCreate::CreateClassComment(const PixelComponent& component)
{
	const PixelClassMeta& meta = component.classComponent;
	std::string content = "---@class " + component.staticComponent.name + "\n";

	for (const auto& member : meta.members)
	{
		if (!member.luaBind)
		{
			continue;
		}
		content += "---@field " + member.name + " " + ToLuaType(component, member.type) + "\n";
	}
	return content + "\n";
}


std::string LuaComponentCreate::CreateMethods(const PixelComponent& component)
{
	const PixelStaticMeta& meta = component.staticComponent;
	std::string className = meta.name;
	std::string content;

	for (const auto& method : meta.methods)
	{
		std::string luaName = CreateLuaFunctionName(className, method.name);
		if (IsStaticComponentMethod(luaName))
		{
			content += CreateStaticMethod(component, method);
		}
		else
		{
			content += CreateDataMethod(component, method);
		}
	}

	return content;
}

std::string LuaComponentCreate::CreateStaticMethod(const PixelComponent& component, const PixelMethodMeta& method)
{
	std::string className = component.staticComponent.name;
	std::string args = CreateArgumentList(method.propertys, false);
	std::string nativeName = CreateNativeFunctionName(className, method.name);
	std::string luaName = CreateLuaFunctionName(className, method.name);
	std::string content;

	if (!method.tooltip.empty())
	{
		content += "--- " + method.tooltip + "\n";
	}

	for (int propertyIndex = 0; propertyIndex < method.propertys.size(); propertyIndex++)
	{
		const PixelParameterMeta& parameter = method.propertys[propertyIndex];
		content += "---@param " + CreateArgumentName(parameter, propertyIndex) + " " + ToLuaType(component, parameter.type);
		if (!parameter.tooltip.empty())
		{
			content += " # " + parameter.tooltip;
		}
		content += "\n";
	}
	if (method.returnType != "void")
	{
		content += "---@return " + ToLuaType(component, method.returnType) + "\n";
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

	return content;
}

std::string LuaComponentCreate::CreateDataMethod(const PixelComponent& component, const PixelMethodMeta& method)
{
	std::string className = component.staticComponent.name;
	std::string args = CreateArgumentList(method.propertys, false, 1);
	std::string nativeArgs = args.empty() ? "self" : "self, " + args;
	std::string nativeName = CreateNativeFunctionName(className, method.name);
	std::string luaName = CreateLuaFunctionName(className, method.name);
	std::string content;

	if (!method.tooltip.empty())
	{
		content += "--- " + method.tooltip + "\n";
	}

	for (int propertyIndex = 1; propertyIndex < method.propertys.size(); propertyIndex++)
	{
		const PixelParameterMeta& parameter = method.propertys[propertyIndex];
		content += "---@param " + CreateArgumentName(parameter, propertyIndex) + " " + ToLuaType(component, parameter.type);
		if (!parameter.tooltip.empty())
		{
			content += " # " + parameter.tooltip;
		}
		content += "\n";
	}
	if (method.returnType != "void")
	{
		content += "---@return " + ToLuaType(component, method.returnType) + "\n";
	}

	content += "function " + component.staticComponent.name + ":" + luaName + "(" + args + ")\n";
	if (method.returnType == "void")
	{
		content += "\tdll." + nativeName + "(" + nativeArgs + ")\n";
	}
	else
	{
		content += "\treturn dll." + nativeName + "(" + nativeArgs + ")\n";
	}
	content += "end\n\n";

	return content;
}

std::string LuaComponentCreate::CreateDataMetatype(const PixelComponent& component)
{
	if (component.classComponent.name.empty() || !HasLuaMembers(component.classComponent))
	{
		return "";
	}

	return "ffi.metatype(\"" + component.classComponent.name + "\", { __index = " + component.staticComponent.name + " })\n\n";
}

std::string LuaComponentCreate::CreateFieldArgumentList(const PixelClassMeta& meta)
{
	std::string args;
	for (const auto& member : meta.members)
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

std::string LuaComponentCreate::CreateFieldInitializers(const PixelClassMeta& meta)
{
	std::string content;
	for (const auto& member : meta.members)
	{
		if (!member.luaBind)
		{
			continue;
		}

		content += "\t\t" + member.name + " = " + member.name + " or " + CreateDefaultValue(member.type) + ",\n";
	}
	return content;
}

std::string LuaComponentCreate::CreateArgumentList(const std::vector<PixelParameterMeta>& parameters, bool includeTypes, int startIndex)
{
	std::string args;
	for (int i = startIndex; i < parameters.size(); i++)
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

std::string LuaComponentCreate::CreateArgumentName(const PixelParameterMeta& parameter, int index)
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

std::string LuaComponentCreate::CreateNativeFunctionName(const std::string& className, const std::string& methodName)
{
	std::string prefix = className + "_";
	if (methodName.rfind(prefix, 0) == 0)
	{
		return methodName;
	}
	return prefix + methodName;
}

std::string LuaComponentCreate::CreateLuaFunctionName(const std::string& className, const std::string& methodName)
{
	std::string prefix = className + "_";
	if (methodName.rfind(prefix, 0) == 0)
	{
		return methodName.substr(prefix.size());
	}
	return methodName;
}

std::string LuaComponentCreate::CreateDefaultValue(const std::string& type)
{
	std::string normalized = NormalizeType(type);
	if (normalized == "bool")
	{
		return "false";
	}
	if (normalized == "constchar*" || normalized == "charconst*" || normalized == "char*" || type == "std::string" || type == "string")
	{
		return "\"\"";
	}
	return "0";
}

std::string LuaComponentCreate::NormalizeType(const std::string& type)
{
	std::string normalized = type;
	std::erase(normalized, ' ');
	return normalized;
}

std::string LuaComponentCreate::ToCType(const std::string& type)
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

std::string LuaComponentCreate::ToLuaType(const std::string& type)
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

std::string LuaComponentCreate::ToLuaType(const PixelComponent& component, const std::string& type)
{
	std::string normalized = ToLuaType(type);
	std::string dataName = NormalizeType(component.classComponent.name);
	std::erase(dataName, '*');
	std::erase(dataName, '&');

	if (normalized == dataName)
	{
		return component.staticComponent.name;
	}

	return normalized;
}

bool LuaComponentCreate::IsStaticComponentMethod(const std::string& luaName)
{
	return luaName.rfind("Get", 0) == 0 ||
		luaName.rfind("Add", 0) == 0 ||
		luaName.rfind("Has", 0) == 0;
}

bool LuaComponentCreate::HasLuaMembers(const PixelClassMeta& meta)
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


/*
void LuaComponentCreate::GenerateLua(const std::string& outPath, PixelClassMeta& meta)
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

bool LuaComponentCreate::ShouldGenerate(PixelClassMeta& meta)
{
	if (meta.metaType != META_TYPE::CLASS) return false;

	if (meta.thisName == "PVector3") return true;
	if (meta.thisName == "TransformData") return true;
	return false;
}

std::string LuaComponentCreate::CreateClassComment(PixelClassMeta& meta, const std::string& className)
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

std::string LuaComponentCreate::CreateCDef(PixelClassMeta& meta, const std::string& className)
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

std::string LuaComponentCreate::CreateConstructor(PixelClassMeta& meta, const std::string& className)
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

std::string LuaComponentCreate::CreateMethods(PixelClassMeta& meta, const std::string& className)
{
	std::string content;
	for (auto& method : meta.methods)
	{
		std::string args = CreateArgumentList(method.propertys, false);
		std::string nativeName = CreateNativeFunctionName(className, method.name);
		std::string luaName = CreateLuaFunctionName(className, method.name);

		if (!method.tooltip.empty())
		{
			content += "--- " + method.tooltip + "\n";
		}
		for (int propertyIndex = 0; propertyIndex < method.propertys.size(); propertyIndex++)
		{
			const PixelParameterMeta& parameter = method.propertys[propertyIndex];
			content += "---@param " + CreateArgumentName(parameter, propertyIndex) + " " + ToLuaType(parameter);
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

std::string LuaComponentCreate::CreateAliases(PixelClassMeta& meta, const std::string& className)
{
	//if (meta.thisName == "PVector3")
	//{
	//	return "Vector3 = Vector3 or " + className + "\n\n";
	//}
	return "";
}

std::string LuaComponentCreate::CreateFieldArgumentList(PixelClassMeta& meta)
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

std::string LuaComponentCreate::CreateFieldInitializers(PixelClassMeta& meta)
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

bool LuaComponentCreate::HasLuaMembers(PixelClassMeta& meta)
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
*/
