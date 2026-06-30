#pragma once
#include "IBind.h"

class LuaStaticCreate : public IBind
{
public:
	LuaStaticCreate();
	virtual ~LuaStaticCreate();
	void Generate(const char* outPath, std::vector<PixelClassMeta>& types) override;

private:
	void GenerateLua(const std::string& outPath, PixelClassMeta& meta);
	bool ShouldGenerate(PixelClassMeta& meta);
	std::string CreateCDef(PixelClassMeta& meta, const std::string& className);
	std::string CreateMethods(PixelClassMeta& meta, const std::string& className);
};
