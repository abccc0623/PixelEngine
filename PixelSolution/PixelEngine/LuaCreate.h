#pragma once
#include "IBind.h"
class LuaCreate : public IBind
{
public:
	LuaCreate();
	virtual ~LuaCreate();
	void Generate(const char* outPath, std::vector<PixelClassMeta>& types) override;
	std::string CreateComponent(PixelClassMeta& meta);
	std::string ComponentFunctionSTR(std::string name);
private:

};

