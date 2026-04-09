#include "pch.h"
#include "LuaBind.h"
#include "PixelMeta.h"
LuaBind::LuaBind()
{

}

LuaBind::~LuaBind()
{

}



void LuaBind::SetIncludeString(PixelClassMeta& meta)
{
    if (meta.metaType == META_TYPE::PRIMITIVE) return;
    if (meta.metaType == META_TYPE::STATIC) return;
    if (meta.metaType == META_TYPE::ENUM) return;
    if (meta.thisName == "Module") return;

    if (meta.thisName == "GameObject")
    {
        IncludeString += ReplaceAll(Include, "INCLUDE_TYPE", "Core/" + meta.thisName);
    }
    else if (meta.thisName == "PVector3")
    {
        IncludeString += ReplaceAll(Include, "INCLUDE_TYPE", "Type/" + meta.thisName);
    }
    else  if (meta.parentHash == GetTypeHashByName("Module"))
    {
        IncludeString += ReplaceAll(Include, "INCLUDE_TYPE", "Module/" + meta.thisName);
    }
    else
    {
        IncludeString += ReplaceAll(Include, "INCLUDE_TYPE", meta.thisName);
    }
}

void LuaBind::SetFunctionString(PixelClassMeta& meta)
{
    if (meta.metaType == META_TYPE::PRIMITIVE) return;

    FunctionCallString += ReplaceAll(FunctionCallName, "TYPE_NAME", meta.thisName);
    FunctionString += ReplaceAll(FunctionName, "TYPE_NAME", meta.thisName);
    FunctionString += "{\n";
    if (meta.metaType == META_TYPE::CLASS || meta.metaType == META_TYPE::STATIC)
    {
        SetLuaClassString(meta);
        SetLuaMemberString(meta);
        SetLuaMethodString(meta);
    }
    else if (meta.metaType == META_TYPE::ENUM)
    {
        SetLuaEnumString(meta);
    }
    FunctionString += "}\n";
}

void LuaBind::SetLuaClassString(PixelClassMeta& meta)
{

    auto it = std::find_if(meta.methods.begin(), meta.methods.end(), [](const auto& method)
        {
            return method.name == "Create"; 
        });
    if (it != meta.methods.end())
    {
        std::unordered_map<std::string, std::string> data;
        data["PROPERTY"] = propertyToString(it->propertys);
        data["CLASS_NAME"] = meta.thisName;
        FunctionString += ReplaceSimple(CreateClassBind, data);
    }
    else if (meta.metaType == META_TYPE::STATIC)
    {
        FunctionString += ReplaceAll(StaticClassBind, "CLASS_NAME", meta.thisName);
    }
    else if (meta.metaType == META_TYPE::CLASS)
    {
        FunctionString += ReplaceAll(ClassBind, "CLASS_NAME", meta.thisName);

        if (meta.parentHash == GetTypeHashByName("Module"))
        {
            AddModuleLuaString += ReplaceAll(LuaAddModuleBind, "CLASS_NAME", meta.thisName);
        }
    }
}

void LuaBind::SetLuaMemberString(PixelClassMeta& meta)
{
    for (auto& m : meta.members)
    {
        std::unordered_map<std::string, std::string> data;
        data["MEMBER_NAME"] = m.name;
        data["CLASS_NAME"]  = meta.thisName;
        FunctionString += ReplaceSimple(ClassMemberBind,data);
    }
}

void LuaBind::SetLuaMethodString(PixelClassMeta& meta)
{
    for (auto& m : meta.methods)
    {
        if (m.name == "Create") 
        {
            continue;
        }

        if (meta.metaType == META_TYPE::STATIC)
        {
            std::unordered_map<std::string, std::string> data;
            data["METHOD_NAME"] = m.name;
            FunctionString += ReplaceSimple(ClassGlobalMethodBind, data);
        }
        else
        {
            std::unordered_map<std::string, std::string> data;
            data["METHOD_NAME"] = m.name;
            data["CLASS_NAME"] = meta.thisName;
            FunctionString += ReplaceSimple(ClassMethodBind, data);
        }
    }
}

void LuaBind::SetLuaEnumString(PixelClassMeta& meta)
{
    FunctionString += ReplaceAll(ClassEnumBind, "ENUM_NAME", meta.thisName);
    for (auto& K : meta.enums) {
        std::unordered_map<std::string, std::string> data;
        data["ENUM_VALUE"] = K.value;
        data["ENUM_NAME"] = meta.thisName;
        FunctionString += ReplaceSimple(ClassEnumValueBind, data);
    }
    FunctionString += "});\n";
}

std::string LuaBind::propertyToString(std::vector<std::string> propertys)
{
    std::string content = "";
    for (int i = 0; i < propertys.size(); i++)
    {
        if (i == propertys.size() - 1)
        {
            content += propertys[i];
        }
        else
        {
            content += propertys[i]+ ",";
        }
    }
    return content;
}


void LuaBind::Generate(const char* outPath, std::vector<PixelClassMeta>& types)
{
    //include 기본
    IncludeString += "#include \"sol.hpp\"\n";
    IncludeString += ReplaceAll(Include,"INCLUDE_TYPE","PixelEngineAPI");
    IncludeString += "#include <unordered_map>\n";

    //FunctionCall함수 제작
    FunctionCallString += "inline void BindAll_GeneratedLuaModules(sol::state& lua)\n";
    FunctionCallString += "{\n";
    FunctionCallString += "\tBindAll_AddModules();\n";

    //AddModule 함수 제작
    AddModuleLuaString += "inline void BindAll_AddModules() \n";
    AddModuleLuaString += "{ \n";

    for (auto& K : types)
    {
        SetIncludeString(K);
        SetFunctionString(K);
    }
    IncludeString += "extern std::unordered_map <std::string, std::function<sol::object(sol::this_state s, Module* target)>> AddModuleList;\n";
    FunctionCallString += "}\n";
    AddModuleLuaString += "}\n";

    std::string main;
    main += IncludeString;
    main += FunctionString;
    main += AddModuleLuaString;
    main += FunctionCallString;

    std::ofstream file(outPath);
    file << main;
    file.close();
}
