#pragma once
#include "IBind.h"
class LuaCreate : public IBind
{
public:
	LuaCreate();
	virtual ~LuaCreate();
	void Generate(const char* outPath, std::vector<PixelClassMeta>& types) override;


private:
	void GenerateLua(const std::string& outPath, PixelClassMeta& meta);
	std::string NormalizeType(const std::string& type);
	std::string ToCType(const std::string& type);
	std::string ToLuaType(const std::string& type);
	std::string CreateArgumentName(const std::string& type, int index);
	std::string CreateArgumentList(const std::vector<std::string>& propertys, bool includeTypes);
	std::string CreateNativeFunctionName(const std::string& className, const std::string& methodName);
	std::string CreateLuaFunctionName(const std::string& className, const std::string& methodName);

private:
	std::string GenerateComponentFileName;
	std::string createFilePath;
};

