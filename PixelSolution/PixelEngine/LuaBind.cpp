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
    if (meta.metaType == META_TYPE::NAMESPACE) return;
    if (meta.thisName == "Module") return;

    if (meta.thisName == "GameObject")
    {
        IncludeString += ReplaceAll(Include, "INCLUDE_TYPE", "Core/" + meta.thisName);
    }
    else if (meta.thisName == "PVector3")
    {
        IncludeString += ReplaceAll(Include, "INCLUDE_TYPE", "Type/" + meta.thisName);
    }
    else  if (meta.parentHash == GetClassHash(GetClass("Module")))
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
    FunctionCallString += ReplaceAll(FunctionCallName, "TYPE_NAME", meta.thisName);
    FunctionString += ReplaceAll(FunctionName, "TYPE_NAME", meta.thisName);
    FunctionString += "{\n";
    SetLuaClassString(meta);
    SetLuaMemberString(meta);
    SetLuaMethodString(meta);
    FunctionString += "}\n";

}

void LuaBind::SetLuaClassString(PixelClassMeta& meta)
{
    if (meta.thisName == "PVector3")
    {
        FunctionString += ReplaceAll(CreateClassBind, "CLASS_NAME", meta.thisName);
    }
    else if (meta.metaType == META_TYPE::NAMESPACE)
    {
        FunctionString += ReplaceAll(StaticClassBind, "CLASS_NAME", meta.thisName);
    }
    else if (meta.metaType == META_TYPE::CLASS)
    {
        FunctionString += ReplaceAll(ClassBind, "CLASS_NAME", meta.thisName);

        if (meta.parentHash == GetClassHash(GetClass("Module")))
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
        if (meta.metaType == META_TYPE::NAMESPACE)
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
