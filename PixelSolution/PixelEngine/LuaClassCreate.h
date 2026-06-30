#pragma once
#include "IBind.h"

class LuaClassCreate : public IBind
{
public:
	LuaClassCreate();
	virtual ~LuaClassCreate();
	void Generate(const char* outPath, std::vector<PixelClassMeta>& types) override;

private:
	void GenerateLua(const std::string& outPath, PixelClassMeta& meta);
	bool ShouldGenerate(PixelClassMeta& meta);
	std::string CreateClassComment(PixelClassMeta& meta, const std::string& className);
	std::string CreateCDef(PixelClassMeta& meta, const std::string& className);
	std::string CreateConstructor(PixelClassMeta& meta, const std::string& className);
	std::string CreateMethods(PixelClassMeta& meta, const std::string& className);
	std::string CreateAliases(PixelClassMeta& meta, const std::string& className);
	std::string CreateFieldArgumentList(PixelClassMeta& meta);
	std::string CreateFieldInitializers(PixelClassMeta& meta);
	bool HasLuaMembers(PixelClassMeta& meta);
};
