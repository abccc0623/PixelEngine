#pragma once
#include "IBind.h"
class LuaCreate : public IBind
{
public:
	LuaCreate();
	virtual ~LuaCreate();
	void Generate(const char* outPath, std::vector<PixelClassMeta>& types) override;
	std::string CreateComponent(PixelClassMeta& meta);

	void ComponentLinkFile();
	void Vector3File();
	void Vector2File();
private:
	std::string GenerateComponentFileName;
};

