#include "pch.h"
#include "LSPBind.h"

void LSPBind::Generate(const char* outPath, std::vector<PixelClassMeta>& types)
{
    main = "";
    for (auto& K : types)
    {
        if (K.metaType == META_TYPE::PRIMITIVE)
        {
            continue;
        }
        else if (K.metaType == META_TYPE::CLASS || K.metaType == META_TYPE::STATIC)
        {
            if (K.parentHash == GetTypeHashByName("Module"))
            {
                std::unordered_map<std::string, std::string> data;
                data["CLASS_NAME"] = K.thisName;
                data["PARENT_NAME"] = "Module";
                main += ReplaceSimple(ClassParentBind, data);
            }
            else
            {
                main += ReplaceAll(ClassBind, "CLASS_NAME", K.thisName);
            }
        
            if (K.thisName == "GameObject")
            {
                SetGameObjectString(types);
                continue;
            }
            SetMemberString(K);
            main += K.thisName +" ={}\n\n";
            SetMethodString(K);
        }
        else if (K.metaType == META_TYPE::ENUM)
        {
            main += ReplaceAll(EnumBind, "ENUM_NAME", K.thisName);
            main += K.thisName + "= {\n";
            for (int i = 0; i < K.enums.size(); i++) 
            {
                main += "\t"+K.enums[i].value + " = " + std::to_string(i) + ",\n";
            }
            main += "}\n";
        }
    }

    std::ofstream file(outPath);
    file << main;
    file.close();
}

std::string LSPBind::TypeChangeByLua(std::string type)
{
    if (type == "int" || type == "float" || type == "double" || type == "char")
    {
        type = "number";
    }
    else if (type == "std::string" || type == "const char*")
    {
        type = "string";
    }
    else if (type == "bool")
    {
        type = "boolean";
    }
    std::erase(type, '*');
    return type;
}

std::string LSPBind::propertyToString(std::vector<std::string> propertys)
{
    std::string content = "";
    for (int i = 0; i < propertys.size(); i++)
    {
        if (i == propertys.size() - 1)
        {
            content += TypeChangeByLua(propertys[i]) + std::to_string(i);
        }
        else
        {
            content += TypeChangeByLua(propertys[i]) + std::to_string(i) + ",";
        }
    }
    return content;
}

void LSPBind::SetMethodString(PixelClassMeta& meta)
{
    for (auto& m : meta.methods) 
    {
        for(int i = 0; i< m.propertys.size();i++)
        {
            std::unordered_map<std::string, std::string> data;
            data["TYPE"] = TypeChangeByLua(m.propertys[i]);
            data["INDEX"] = std::to_string(i);
            main += ReplaceSimple(PropertyBind, data);
        }
        main += ReplaceAll(ReturnBind, "TYPE", TypeChangeByLua(m.returnType));

        std::unordered_map<std::string, std::string> data;
        data["CLASS_NAME"] = meta.thisName;
        data["FUNCTION_NAME"] = m.name;
        data["PROPERTY"] = propertyToString(m.propertys);

        if (m.name == "Create")
        {
            data["FUNCTION_NAME"] = "new";
            main += ReplaceSimple(StaticMethodBind, data);
        }
        else
        {
            if (meta.metaType == META_TYPE::CLASS)
            {
                main += ReplaceSimple(MethodBind, data);
            }
            else if (meta.metaType == META_TYPE::STATIC)
            {
                main += ReplaceSimple(StaticMethodBind, data);
            }
        }
    }
}

void LSPBind::SetMemberString(PixelClassMeta& meta)
{
    for (auto& m : meta.members)
    {
        std::unordered_map<std::string, std::string> data;
        data["NAME"] = m.name;
        data["TYPE"] = TypeChangeByLua(m.type);
        main += ReplaceSimple(MemberBind, data);
    }
}

void LSPBind::SetGameObjectString(std::vector<PixelClassMeta>& types)
{
    main += "GameObject = {} \n";
    for (auto& K : types)
    {
        if (K.parentHash == GetTypeHashByName("Module"))
        {
            main += ReplaceAll(OverloadObject, "CLASS_NAME", K.thisName);
        }
    }
    main += "---@param string0 string\n";
    main += "---@return Module\n";
    main += "function GameObject:AddModule(string0) end\n\n";


    for (auto& K : types)
    {
        if (K.parentHash == GetTypeHashByName("Module"))
        {
            main += ReplaceAll(OverloadObject, "CLASS_NAME", K.thisName);
        }
    }
    main += "---@param string0 string\n";
    main += "---@return Module\n";
    main += "function GameObject:GetModule(string0) end\n\n";
}
