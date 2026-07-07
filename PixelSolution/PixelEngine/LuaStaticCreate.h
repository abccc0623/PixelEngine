#pragma once
#include "PixelMeta.h"
#include "IBind.h"

class LuaStaticCreate : IBind
{
public:
	LuaStaticCreate();
	virtual ~LuaStaticCreate();
	void Generate(const std::string& outPath, std::map<std::string, PixelClassMeta>& staticClass);
private:
	std::string CreateFunction(const PixelClassMeta& PClass) override;
};
