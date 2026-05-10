#pragma once
#include "PixelMeta.h"
#include <vector>
#include <unordered_map>
class IBind
{
public:
	IBind() = default;
	~IBind() = default;

	virtual void Generate(const char* outPath, std::vector<PixelClassMeta>& types) = 0;
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
};