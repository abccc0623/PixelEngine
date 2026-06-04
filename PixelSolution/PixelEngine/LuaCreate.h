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
	std::string TypeChangeByLua(const std::string& type);
	std::string CreateMethodWrapper(PixelClassMeta& meta, PixelMethodMeta& method);
	std::string CreatePropertyList(const std::vector<std::string>& propertys);
	std::string CreatePropertyName(const std::string& type, int index);
private:
	std::string GenerateComponentFileName;
};

