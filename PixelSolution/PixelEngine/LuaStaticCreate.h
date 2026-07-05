#pragma once
#include "PixelMeta.h"
#include "IBind.h"

class LuaStaticCreate : IBind
{
public:
	LuaStaticCreate();
	virtual ~LuaStaticCreate();
	void Generate(const std::string& outPath, std::map<std::string, PixelClassMeta>& staticClass);
	bool HasGeneratedMethods(const PixelStaticMeta& meta);

private:
	std::string CreateFunction(const PixelClassMeta& PClass) override;

	std::string CreateMethods(const PixelStaticMeta& meta);
	std::string CreateArgumentList(const std::vector<PixelParameterMeta>& parameters, bool includeTypes);
	std::string CreateArgumentName(const PixelParameterMeta& parameter, int index);
	std::string CreateNativeFunctionName(const std::string& className, const std::string& methodName);
	std::string CreateLuaFunctionName(const std::string& className, const std::string& methodName);
	std::string NormalizeType(const std::string& type);
	std::string ToCType(const std::string& type);
	std::string ToLuaType(const std::string& type);
	bool ShouldGenerateMethod(const PixelStaticMeta& meta, const PixelMethodMeta& method);
	bool IsExportedNativeMethod(const std::string& methodName);
	bool IsSupportedFFIType(const std::string& type);
	bool IsStaticComponentMethod(const std::string& luaName);
};
