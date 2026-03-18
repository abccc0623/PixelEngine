#pragma once
#include <string>

const std::string& CLASS_NAME	= "{{CLASS_NAME}}";
const std::string& MEMBER_NAME	= "{{MEMBER_NAME}}";
const std::string& METHOD_NAME	= "{{METHOD_NAME}}";
const std::string& INCLUDE_PATH = "{{INCLUDE_PATH}}";
const std::string& KEY_HASH = "{{INCLUDE_PATH}}";

//LuaBind필요한거...
std::string IncludeString		= "#include \"{{INCLUDE_PATH}}\" \n";
std::string LuaFunctionString	= "inline void Generate_{{CLASS_NAME}}(sol::state& lua) \n";
std::string LuaClassBindString	= "\tsol::usertype<{{CLASS_NAME}}> ut = lua.new_usertype<{{CLASS_NAME}}>(\"{{CLASS_NAME}}\");\n";
std::string LuaMemberBindString	= "\tut[\"{{MEMBER_NAME}}\"] = &{{CLASS_NAME}}::{{MEMBER_NAME}};\n";
std::string LuaMethodBindString = "\tut[\"{{METHOD_NAME}}\"] = &{{CLASS_NAME}}::{{METHOD_NAME}};\n";
std::string LuaCallBindString	= "\tGenerate_{{CLASS_NAME}}(lua);\n";


std::string LuaAddModuleString = "\tAddModuleList.insert({ \"{{CLASS_NAME}}\",[](sol::this_state s, Module* target) -> sol::object{sol::object obj = sol::make_object(s, static_cast<{{CLASS_NAME}}* > (target));return obj;}});\n";


