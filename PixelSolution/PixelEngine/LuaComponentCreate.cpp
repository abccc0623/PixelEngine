#include "pch.h"
#include "LuaComponentCreate.h"
#include "PixelMetaAPI.h"
#include <filesystem>
#include <fstream>

LuaComponentCreate::LuaComponentCreate()
{}

LuaComponentCreate::~LuaComponentCreate()
{}

void LuaComponentCreate::Generate(const std::string& outPath, std::map<std::string, PixelClassMeta>& component)
{
	for (const auto& meta : component)
	{
		std::string content;
		content += "local ffi = require(\"ffi\")\n";
		content += "local dll = ffi.load(\"PixelEngine\")\n\n";
		content += "ffi.cdef[[\n";
		content += CreateCDef(meta.second);
		content += "]]\n\n";
		content += CreateClassAnnotation(meta.second);
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
	content += "\tlocal data = dll.{{CLASS_NAME}}_Add(ID)\n";
	content += "\tdata.thisID = ID\n";
	content += "\treturn data \n";
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

	content += "local " + PClass.name + "Data = {}\n\n";

	for (int i = 0; i < PClass.methods.size(); i++)
	{
		if (PClass.methods[i].name == PClass.name + "_Add" ||
			PClass.methods[i].name == PClass.name + "_Get" ||
			PClass.methods[i].name == PClass.name + "_Has")
		{
			continue;
		}

		size_t pos = PClass.methods[i].name.find('_');
		std::string left = PClass.methods[i].name.substr(0, pos);
		std::string right = PClass.methods[i].name.substr(pos + 1);
		const auto& method = PClass.methods[i];
		const bool usesComponentID = !method.propertys.empty() &&
			(method.propertys[0].name == "ID" || method.propertys[0].name == "id");

		std::string luaParameters;
		std::string nativeParameters;
		if (usesComponentID)
		{
			nativeParameters = "self.thisID";
		}

		const int firstLuaParameter = usesComponentID ? 1 : 0;
		for (int parameterIndex = firstLuaParameter; parameterIndex < method.propertys.size(); parameterIndex++)
		{
			if (!luaParameters.empty())
			{
				luaParameters += ",";
			}
			luaParameters += method.propertys[parameterIndex].name;
			if (!nativeParameters.empty())
			{
				nativeParameters += ",";
			}
			nativeParameters += method.propertys[parameterIndex].name;
		}

		for (int parameterIndex = firstLuaParameter; parameterIndex < method.propertys.size(); parameterIndex++)
		{
			content += "---@param " + method.propertys[parameterIndex].name + " " +
				ToLuaType(method.propertys[parameterIndex].type) + "\n";
		}
		if (!method.returnType.empty() && method.returnType != "void")
		{
			content += "---@return " + ToLuaType(method.returnType) + "\n";
		}
		content += "function " + PClass.name + "Data:" + right + "(" + luaParameters + ")\n";
		content += "\t";
		if (!method.returnType.empty())
		{
			content += "return ";
		}
		content += "dll." + method.name + "(" + nativeParameters + ")\n";
		content += "end\n\n";
	}

	content += "local " + PClass.name + "_mt = {\n";
	content += "\t__index = " + PClass.name + "Data\n";
	content += "}\n\n";
	return content;
}
