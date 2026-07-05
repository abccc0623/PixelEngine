#pragma once
#include "PixelMeta.h"
#include <vector>
#include <unordered_map>
#include "EngineMetaFlag.h"
class IBind
{
public:
	IBind() = default;
	~IBind() = default;
protected:
	std::string ApplyTemplate(const std::string& templateStr, const std::unordered_map<std::string, std::string>& values)
	{
		std::string result = templateStr;
		std::vector<std::pair<std::string, std::string>> sorted(
			values.begin(), values.end());
		std::sort(sorted.begin(), sorted.end(),
			[](const auto& a, const auto& b)
			{
				return a.first.length() > b.first.length();
			});

		for (const auto& [key, value] : sorted)
		{
			std::string token = "{{" + key + "}}";
			size_t pos = 0;
			while ((pos = result.find(token, pos)) != std::string::npos)
			{
				result.replace(pos, token.length(), value);
				pos += value.length();
			}
		}
		return result;
	}

	std::string ReplaceAll(
		const std::string& str,
		const std::string& key,
		const std::string& value)
	{
		std::string result = str;
		std::string token = "{{" + key + "}}";

		size_t pos = 0;
		while ((pos = result.find(token, pos)) != std::string::npos)
		{
			result.replace(pos, token.length(), value);
			pos += value.length(); // 무한루프 방지
		}

		return result;
	}

	std::string ReplaceSimple(
		const std::string& templateStr,
		const std::unordered_map<std::string, std::string>& values)
	{
		std::string result = templateStr;

		for (const auto& [key, value] : values)
		{
			std::string token = "{{" + key + "}}";

			size_t pos = 0;
			while ((pos = result.find(token, pos)) != std::string::npos)
			{
				result.replace(pos, token.length(), value);
				pos += value.length();
			}
		}

		return result;
	}
	std::string NormalizeType(const std::string& type)
	{
		std::string normalized = type;
		std::erase(normalized, ' ');
		return normalized;
	}

	std::string ToCType(const std::string& type)
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

	std::string ToLuaType(const std::string& type)
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

	std::string CreateArgumentName(const std::string& type, int index)
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

	std::string CreateArgumentList(const std::vector<std::string>& propertys, bool includeTypes)
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

	std::string CreateNativeFunctionName(const std::string& className, const std::string& methodName)
	{
		std::string prefix = className + "_";
		if (methodName.rfind(prefix, 0) == 0)
		{
			return methodName;
		}
		return prefix + methodName;
	}

	std::string CreateLuaFunctionName(const std::string& className, const std::string& methodName)
	{
		std::string prefix = className + "_";
		if (methodName.rfind(prefix, 0) == 0)
		{
			return methodName.substr(prefix.size());
		}
		return methodName;
	}

	std::string CreateDefaultValue(const std::string& type)
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

	std::string CreateCDef(const PixelClassMeta& meta);
	std::string CreateClassAnnotation(const PixelClassMeta& meta);

	std::string CreateMethodParameter(const PixelMethodMeta& meta);
	std::string CreateMethod(const PixelMethodMeta& meta);
	virtual std::string CreateFunction(const PixelClassMeta& PClass);
};