#pragma once
#include "PixelMeta.h"

class LuaEnumCreate
{
public:
	LuaEnumCreate();
	virtual ~LuaEnumCreate();
	void Generate(const std::string& outPath, std::map<std::string, PixelEnumMeta>& enumlist);

private:
	void GenerateLua(const std::string& outPath, const PixelEnumMeta& meta);
	std::string CreateCDef(const PixelEnumMeta& meta);
	std::string CreateEnumTable(const PixelEnumMeta& meta);
};
