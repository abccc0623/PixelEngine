#pragma once
#include "IBind.h"
class LuaBind : public IBind
{
public:
	LuaBind();
	~LuaBind();
	void Generate(const char* outPath, std::vector<PixelClassMeta>& types) override;
private:
	void SetIncludeString(PixelClassMeta& meta);
	void SetFunctionString(PixelClassMeta& meta);
	void SetLuaClassString(PixelClassMeta& meta);
	void SetLuaMemberString(PixelClassMeta& meta);
	void SetLuaMethodString(PixelClassMeta& meta);
	void SetLuaEnumString(PixelClassMeta& meta);


	std::string propertyToString(std::vector<std::string> propertys);

	std::string IncludeString;
	std::string FunctionString;
	std::string FunctionCallString;
	std::string AddModuleLuaString;

	const std::string Include = "#include \"{{INCLUDE_TYPE}}.h\" \n";
	const std::string FunctionName = "inline void Generate_{{TYPE_NAME}}(sol::state& lua) \n";
	const std::string FunctionCallName = "\tGenerate_{{TYPE_NAME}}(lua); \n";
	const std::string ClassBind = "\tsol::usertype<{{CLASS_NAME}}> ut = lua.new_usertype<{{CLASS_NAME}}>(\"{{CLASS_NAME}}\");\n";
	const std::string StaticClassBind = "\tsol::table ut = lua.create_named_table(\"{{CLASS_NAME}}\");\n";
	const std::string CreateClassBind = "\tsol::usertype<{{CLASS_NAME}}> ut = lua.new_usertype<{{CLASS_NAME}}>(\"{{CLASS_NAME}}\",sol::constructors<{{CLASS_NAME}}({{PROPERTY}})>()); \n";
	const std::string ClassMemberBind = "\tut[\"{{MEMBER_NAME}}\"] = &{{CLASS_NAME}}::{{MEMBER_NAME}};\n";
	const std::string ClassMethodBind = "\tut[\"{{METHOD_NAME}}\"] = &{{CLASS_NAME}}::{{METHOD_NAME}};\n";
	const std::string ClassEnumBind = "\tlua.new_enum<{{ENUM_NAME}}>(\"{{ENUM_NAME}}\", {\n";
	const std::string ClassEnumValueBind = "\t{ \"{{ENUM_VALUE}}\", {{ENUM_NAME}}::{{ENUM_VALUE}} },\n";
	const std::string ClassGlobalMethodBind = "\tut[\"{{METHOD_NAME}}\"] = &{{METHOD_NAME}};\n";
	const std::string ClassGlobalMethodBindComponent = "\tut[\"{{METHOD_NAME}}\"] = &{{METHOD_NAMESPACE}};\n";
	const std::string LuaAddModuleBind = "\tAddModuleList.insert({ \"{{CLASS_NAME}}\",[](sol::this_state s, Module* target) -> sol::object{sol::object obj = sol::make_object(s, static_cast<{{CLASS_NAME}}* > (target));return obj;}});\n";
};

