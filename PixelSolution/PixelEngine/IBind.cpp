#include "pch.h"
#include "IBind.h"

std::string IBind::CreateCDef(const PixelClassMeta& meta)
{
	std::string content;
	if (meta.members.size() != 0)
	{
		std::string member = "\ttypedef struct {\n";
		for (int i = 0; i < meta.members.size(); i++)
		{
			meta.members[i].type;
			meta.members[i].name;
			member += "\t\t" + meta.members[i].type + " " + meta.members[i].name + ";\n";
		}

		if ((meta.flag & EngineMetaFlag::Component) != 0)
		{
			member += "\t} " + meta.name + "Data" + ";\n\n";
		}
		else
		{
			member += "\t} " + meta.name + ";\n\n";
		}
		content += member;
	}

	if (meta.methods.size() != 0)
	{
		for (const auto& method : meta.methods)
		{
			std::string returnSTR = method.returnType;
			std::string functionNameSTR = method.name;
			std::string propertySTR = "";
			int propertysCount = method.propertys.size();
			for (int i = 0; i < propertysCount; i++)
			{
				std::string propertyName = method.propertys[i].name;
				std::string propertyType = method.propertys[i].type;
				std::string propertyTip = method.propertys[i].tooltip;

				propertySTR += propertyType + " " + propertyName;
				if (i != propertysCount - 1)
				{
					propertySTR += ",";
				}
			}
			content += "\t" + returnSTR + " " + functionNameSTR + "(" + propertySTR + ");\n";
		}
	}
	return content;
}

std::string IBind::CreateClassAnnotation(const PixelClassMeta& meta)
{
	std::string content;
	const bool isComponent =
		(meta.flag & (EngineMetaFlag::Component | EngineMetaFlag::ComponentData)) != 0;

	if (isComponent)
	{
		content += "---@class " + meta.name + "Data\n";
	}
	else
	{
		content += "---@class " + meta.name + "\n";
	}
	for (int i = 0; i < meta.members.size(); i++)
	{
		if (meta.members[i].flag == EngineMetaFlag::Private) continue;
		content += "---@field " + meta.members[i].name + " " + ToLuaType(meta.members[i].type) + "\n";
	}

	return content;
}

std::string IBind::CreateMethodParameter(const PixelMethodMeta& meta)
{
	std::string propertySTR = "";
	int propertysCount = meta.propertys.size();
	for (int i = 0; i < propertysCount; i++)
	{
		std::string propertyName = meta.propertys[i].name;
		propertySTR += propertyName;
		if (i != propertysCount - 1)
		{
			propertySTR += ",";
		}
	}
	return propertySTR;
}

std::string IBind::CreateMethod(const PixelMethodMeta& meta)
{
	std::string function = "";
	if (meta.returnType != "")
	{
		function += "return ";
	}
	function += "dll." + meta.name + "(" + CreateMethodParameter(meta) + ")\n";
	return function;
}

std::string IBind::CreateFunction(const PixelClassMeta& PClass)
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
