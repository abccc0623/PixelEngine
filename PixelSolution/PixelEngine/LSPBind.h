#pragma once
#include "IBind.h"
class LSPBind :public IBind
{
public:
	LSPBind() = default;
	~LSPBind() = default;
	void Generate(const char* outPath, std::vector<PixelClassMeta>& types) override;
private:
	std::string TypeChangeByLua(std::string type);
	std::string propertyToString(std::vector<std::string> propertys);
	void SetMethodString(PixelClassMeta& meta);
	void SetMemberString(PixelClassMeta& meta);
	void SetGameObjectString();



	std::string main = "";
	const std::string ClassBind		= "---@class {{CLASS_NAME}}\n";
	const std::string PropertyBind	= "---@param {{TYPE}}{{INDEX}} {{TYPE}}\n";
	const std::string ReturnBind	= "---@return {{TYPE}}\n";
	const std::string MemberBind	= "---@field {{NAME}} {{TYPE}}\n";
	const std::string MethodBind		= "function {{CLASS_NAME}}:{{FUNCTION_NAME}}({{PROPERTY}}) end\n\n";
	const std::string StaticMethodBind	= "function {{CLASS_NAME}}.{{FUNCTION_NAME}}({{PROPERTY}}) end\n\n";

	const std::string CreateClassBind = "---@return {{CLASS_NAME}} \n function {{CLASS_NAME}}.new() end\n";
};

