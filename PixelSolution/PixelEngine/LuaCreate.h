#pragma once
#include "IBind.h"
class LuaCreate : public IBind
{
public:
	LuaCreate();
	virtual ~LuaCreate();
	void Generate(const char* outPath, std::vector<PixelClassMeta>& types) override;
	std::string CreateComponent(PixelClassMeta& meta, std::vector<PixelClassMeta>& types);

	void ComponentLinkFile();
	void Vector3File();
	void Vector2File();
private:
	PixelClassMeta* FindType(std::vector<PixelClassMeta>& types, const std::string& name);
	std::string CreateCDef(PixelClassMeta& meta, std::vector<PixelClassMeta>& types);
	std::string TypeChangeByCType(const std::string& type);
private:
	std::string GenerateComponentFileName;
};

