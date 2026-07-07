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

		size_t pos = PClass.methods[i].name.find('_');
		std::string left = PClass.methods[i].name.substr(0, pos);
		std::string right = PClass.methods[i].name.substr(pos + 1);

		content += "\t\t" + right + " = function(" + CreateMethodParameter(PClass.methods[i]) + ")\n";
		content += "\t\t\t" + CreateMethod(PClass.methods[i]);
		content += "\t\tend\n";
	}

	content += "\t}\n";
	content += "}\n\n";
	return content;
}
