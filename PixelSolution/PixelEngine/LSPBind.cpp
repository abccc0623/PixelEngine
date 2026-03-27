#include "pch.h"
#include "LSPBind.h"

void LSPBind::Generate(const char* outPath, std::vector<PixelClassMeta>& types)
{
    main = "";
    for (auto& K : types)
    {
        main += ReplaceAll(ClassBind, "CLASS_NAME", K.thisName);
        if (K.thisName == "GameObject")
        {
            SetGameObjectString();
            continue;
        }
        SetMemberString(K);
        main += K.thisName +" ={}\n\n";
        SetMethodString(K);
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

        if (meta.metaType == META_TYPE::CLASS)
        {
            main += ReplaceSimple(MethodBind, data);
        }
        else if (meta.metaType == META_TYPE::NAMESPACE)
        {
            main += ReplaceSimple(StaticMethodBind, data);
        }
    }

    if (meta.thisName == "PVector3")
    {
        main += ReplaceAll(CreateClassBind,"CLASS_NAME", meta.thisName);
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

void LSPBind::SetGameObjectString()
{
    main += "GameObject = {} \n";
    main += "--- @generic T \n";
    main += "--- @param ModuleName `T` \n";
    main += "--- @return T \n";
    main += "function GameObject:AddModule(ModuleName) end \n\n";
    main += "--- @generic T \n";
    main += "--- @param ModuleName `T` \n";
    main += "--- @return T \n";
    main += "function GameObject:GetModule(ModuleName) end \n\n";
}
