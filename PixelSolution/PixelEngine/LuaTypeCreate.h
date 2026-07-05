#pragma once
#include "PixelMeta.h"

class LuaTypeCreate
{
public:
	LuaTypeCreate();
	virtual ~LuaTypeCreate();
	void Generate(const std::string& outPath, std::vector<PixelClassMeta>& classList);

private:
	void GenerateLua(const std::string& outPath, const PixelClassMeta& meta);
	std::string CreateClassComment(const PixelClassMeta& meta);
	std::string CreateCDef(const PixelClassMeta& meta);
	std::string CreateMetatype(const PixelClassMeta& meta);
	std::string NormalizeType(const std::string& type);
	std::string ToCType(const std::string& type);
	std::string ToLuaType(const std::string& type);
	bool ShouldGenerate(const PixelClassMeta& meta);
	bool HasLuaMembers(const PixelClassMeta& meta);
};
