#pragma once
#include "IBind.h"

class LuaComponentCreate : IBind
{
public:
	LuaComponentCreate();
	virtual ~LuaComponentCreate();
	void Generate(const std::string& outPath, std::map<std::string, PixelClassMeta>& component);
private:
	std::string CreateBaseFunction(const PixelClassMeta& PClass);
	std::string CreateFunction(const PixelClassMeta& PClass) override;
};
