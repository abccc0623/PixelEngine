#pragma once
#include "PixelMeta.h"
#include "IBind.h"
class LuaTypeCreate : IBind
{
public:
	LuaTypeCreate();
	virtual ~LuaTypeCreate();
	void Generate(const std::string& outPath, std::map<std::string, PixelClassMeta>& classList);
private:
	std::string CreateFunction(const PixelClassMeta& PClass) override;
};
