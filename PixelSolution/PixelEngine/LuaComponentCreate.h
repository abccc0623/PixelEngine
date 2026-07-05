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



	std::string CreateClassComment(const PixelComponent& component);
	std::string CreateMethods(const PixelComponent& component);
	std::string CreateStaticMethod(const PixelComponent& component, const PixelMethodMeta& method);
	std::string CreateDataMethod(const PixelComponent& component, const PixelMethodMeta& method);
	std::string CreateDataMetatype(const PixelComponent& component);
	std::string CreateFieldArgumentList(const PixelClassMeta& meta);
	std::string CreateFieldInitializers(const PixelClassMeta& meta);
	std::string CreateArgumentList(const std::vector<PixelParameterMeta>& parameters, bool includeTypes, int startIndex = 0);
	std::string CreateArgumentName(const PixelParameterMeta& parameter, int index);
	std::string CreateNativeFunctionName(const std::string& className, const std::string& methodName);
	std::string CreateLuaFunctionName(const std::string& className, const std::string& methodName);
	std::string CreateDefaultValue(const std::string& type);
	std::string NormalizeType(const std::string& type);
	std::string ToCType(const std::string& type);
	std::string ToLuaType(const std::string& type);
	std::string ToLuaType(const PixelComponent& component, const std::string& type);
	bool IsStaticComponentMethod(const std::string& luaName);
	bool HasLuaMembers(const PixelClassMeta& meta);
};
