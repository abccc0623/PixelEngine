#include "pch.h"
#include "GenerateManager.h"
#include "PixelMetaAPI.h"
#include "LuaGenerateTemplate.h"
using json = nlohmann::json;
void GenerateManager::Initialize()
{

}

void GenerateManager::Update()
{

}

void GenerateManager::Release()
{

}

void GenerateManager::Clear()
{

}

void GenerateManager::Generate(PClass* target, int index, std::string typeName)
{
    //인클루드 설정
    if (GetClassParentHash(target) == GetClassHashByString("Module"))
    {
        //부모가 Module인 경우
        includeSTR += ChangeString("{{INCLUDE_TYPE}}", typeName, IncludeModuleString);
        std::unordered_map<std::string, std::string> replaceData =
        {
            {"{{CLASS_NAME}}", typeName},
        };
        addModuleSTR += ChangeString(replaceData,LuaAddModuleString);
    }
    else if (GetClassHash(target) == GetClassHashByString("GameObject"))
    {
        //Core형태의 클래스일 경우
        includeSTR += ChangeString("{{INCLUDE_TYPE}}", typeName, IncludeCoreString);
    }
    else if(GetClassHash(target) == GetClassHashByString("PVector3"))
    {
        //Type형태의 클래스일 경우
        includeSTR += ChangeString("{{INCLUDE_TYPE}}", typeName, IncludeTypesString);
    }
    functionSTR     += ChangeString("{{CLASS_NAME}}", typeName, LuaCallBindString);
    contentSTR      += ChangeString("{{CLASS_NAME}}", typeName, LuaFunctionString);
    contentSTR  += "{ \n";

    //진짜 클래스 파싱
    uint64_t hash = GetClassHash(target);
    int memberCount = GetClassMemberCount(target);
    int methodCount = GetClassMethodCount(target);
    contentSTR += ChangeString("{{CLASS_NAME}}", typeName, LuaClassBindString);
    for (int m = 0; m < memberCount; m++)
    {
        std::string memberName = GetClassMemberName(target, m);
        std::string memberType = GetClassMemberType(target, m);
        std::unordered_map<std::string, std::string> replaceData =
        {
            {"{{CLASS_NAME}}", typeName},
            {"{{MEMBER_NAME}}", memberName}
        };
        contentSTR += ChangeString(replaceData, LuaMemberBindString);
    }
    //함수 등록 
    for (int m = 0; m < methodCount; m++)
    {
        std::string methodName = GetClassMethodName(target, m);

        if (HasClassMethodFlag(target, m, MetaFlag::LUABIND) == false) continue;
        std::unordered_map<std::string, std::string> replaceData =
        {
            {"{{CLASS_NAME}}", typeName},
            {"{{METHOD_NAME}}", methodName}
        };
        contentSTR += ChangeString(replaceData, LuaMethodBindString);
    }
    contentSTR  += "} \n\n";
}

void GenerateManager::Generate(PNamespace* target, int index, std::string typeName)
{
    functionSTR += ChangeString("{{CLASS_NAME}}", typeName, LuaCallBindString);
    contentSTR += ChangeString("{{CLASS_NAME}}", typeName, LuaFunctionString);
    contentSTR += "{\n";
    contentSTR += ChangeString("{{CLASS_NAME}}", typeName, LuaGlobalBindString);

    int namespaceCount = GetNamespaceMethodCount(target);
    for (int m = 0; m < namespaceCount; m++)
    {
        std::string methodName = GetNamespaceMethodName(target, m);
    
        if (HasNamespaceMethodFlag(target, m, MetaFlag::LUABIND) == false) continue;
        std::unordered_map<std::string, std::string> replaceData =
        {
            {"{{METHOD_NAME}}", methodName}
        };
        contentSTR += ChangeString(replaceData, LuaGlobalMemberBindString);
    }
    contentSTR += "}\n\n";
}

void GenerateManager::CreateLuaBindCode(const char* outPath)
{
    functionSTR += "inline void BindAll_GeneratedLuaModules(sol::state& lua) \n";
    functionSTR += "{\n";
    functionSTR += "\tBindAll_AddModules(); \n";
    addModuleSTR += "inline void BindAll_AddModules()\n";
    addModuleSTR += "{\n";
    globalContentSTR += "inline void Generate_Global(sol::state& lua)\n";
    globalContentSTR += "{\n";
    contentSTR = "";

    //기본적으로 필요한 인클루드
    includeSTR = "";
    includeSTR += "#include \"sol.hpp\" \n";
    includeSTR += "#include \"PixelEngineAPI.h\" \n";

    int allCount =  GetTypeAllCount();
    //모든 타입을 순회
    for (int typeIndex = 0; typeIndex < allCount; typeIndex++)
    {
       PType* targetType = GetTypeByIndex(typeIndex);
       auto metaType = GetTypeCategory(targetType);
       const std::string& targetName = GetTypeName(targetType);
       switch (metaType)
       {
        case META_TYPE::CLASS:
            Generate(GetClass(targetName), typeIndex,targetName);
            break;
        case META_TYPE::NAMESPACE:
            Generate(GetNamespace(targetName), typeIndex, targetName);
            break;
        case META_TYPE::PRIMITIVE:
            break;
       }
    }
    includeSTR += "#include <unordered_map>\n";
    includeSTR += "extern std::unordered_map <std::string, std::function<sol::object(sol::this_state s, Module* target)>> AddModuleList;\n";
    functionSTR += "}\n";
    addModuleSTR += "}\n";
    globalContentSTR += "}\n";
    std::string main = "";
    main += includeSTR;
    main += contentSTR;
    main += addModuleSTR;
    main += globalContentSTR;
    main += functionSTR;

   std::ofstream outFile(outPath);
   if (!outFile.is_open()) return;
   outFile << main;
   outFile.close();
}

std::string GenerateManager::ChangeString(const std::string& command, const std::string& changeName, const std::string& templateStr)
{
    // 리턴할 결과물만 한 번 복사합니다.
    std::string resultTemplate = templateStr;

    size_t pos = resultTemplate.find(command);
    while (pos != std::string::npos)
    {
        resultTemplate.replace(pos, command.length(), changeName);
        pos = resultTemplate.find(command, pos + changeName.length());
    }

    return resultTemplate;
}
std::string GenerateManager::ChangeString(const std::unordered_map<std::string, std::string>& replacements, const std::string& templateStr)
{
    std::string resultTemplate = templateStr;
    // 맵에 들어있는 모든 치환 쌍(명령어, 바꿀이름)에 대해 루프를 돕니다.
    for (const auto& pair : replacements)
    {
        const std::string& command = pair.first;      // 예: "{{CLASS_NAME}}"
        const std::string& changeName = pair.second;  // 예: "Player"

        size_t pos = resultTemplate.find(command);
        while (pos != std::string::npos)
        {
            resultTemplate.replace(pos, command.length(), changeName);
            pos = resultTemplate.find(command, pos + changeName.length());
        }
    }
    return resultTemplate;
}


void GenerateManager::GenerateLua(PClass* target, int index, std::string typeName)
{
    generateMetaFiles += "\n";
    generateMetaFiles += "---@class " + typeName +"\n";

    if (typeName == "GameObject")
    {
        generateMetaFiles += "GameObject = {} \n";
        generateMetaFiles += "---@generic T \n";
        generateMetaFiles += "---@param arg1 `T` \n";
        generateMetaFiles += "---@return T \n";
        generateMetaFiles += "function GameObject:AddModule(arg1) end \n";

        generateMetaFiles += "---@generic T \n";
        generateMetaFiles += "---@param arg1 `T` \n";
        generateMetaFiles += "---@return T \n";
        generateMetaFiles += "function GameObject:GetModule(arg1) end \n";
        return;
    }


    uint64_t hash = GetClassHash(target);
    int memberCount = GetClassMemberCount(target);
    int methodCount = GetClassMethodCount(target);

    for (int m = 0; m < memberCount; m++)
    {
        std::string memberName = GetClassMemberName(target, m);
        std::string memberType = GetClassMemberType(target, m);
        memberType = TypeChangeByLua(memberType);
        generateMetaFiles += "---@field " + memberName+" " + memberType +"\n";
    }
    generateMetaFiles += typeName + " ={}\n";

    for (int m = 0; m < methodCount; m++)
    {
        std::string methodName = GetClassMethodName(target, m);
        if (HasClassMethodFlag(target, m, MetaFlag::LUABIND) == false) continue;
       
        std::string Property = "";
        std::string ReturnType = GetClassMethodReturnType(target, m);
        std::string MethodName = GetClassMethodName(target, m);

        int PropertyCount = GetClassMethodPropertyCount(target, m);

        ReturnType = TypeChangeByLua(ReturnType);
        for (int p = 0; p < PropertyCount; p++)
        {
            std::string TypeName = GetClassMethodGetPropertyType(target, m, p);
            TypeName = TypeChangeByLua(TypeName);
            if (TypeName == "sol::this_state")continue;

            generateMetaFiles += "---@param arg"+ std::to_string(p)+ " "+ TypeName + "\n";
            if (p == PropertyCount - 1)
            {
                Property += "arg" + std::to_string(p);
            }
            else
            {
                Property += "arg" + std::to_string(p) + ",";
            }

        }
        generateMetaFiles += "---@return " + ReturnType + "\n";
        generateMetaFiles += "function " + typeName + ":" + MethodName+ "(" + Property +") end\n\n";
    }
}

void GenerateManager::GenerateLua(PNamespace* target, int index, std::string typeName)
{
    generateMetaFiles += "\n";
    generateMetaFiles += "---@class " + typeName + "\n";
    generateMetaFiles += typeName + " ={}\n";
    int methodCount = GetNamespaceMethodCount(target);

    for (int m = 0; m < methodCount; m++)
    {
        std::string methodName = GetNamespaceMethodName(target, m);
        if (HasNamespaceMethodFlag(target, m, MetaFlag::LUABIND) == false) continue;

        std::string Property = "";
        std::string ReturnType = GetNamespaceMethodReturnType(target, m);
        std::string MethodName = GetNamespaceMethodName(target, m);

        int PropertyCount = GetNamespaceMethodPropertyCount(target, m);

        ReturnType = TypeChangeByLua(ReturnType);
        for (int p = 0; p < PropertyCount; p++)
        {
            std::string TypeName = GetNamespaceMethodGetPropertyType(target, m, p);
            TypeName = TypeChangeByLua(TypeName);
            if (TypeName == "sol::this_state")continue;

            generateMetaFiles += "---@param arg" + std::to_string(p) + " " + TypeName + "\n";
            if (p == PropertyCount - 1)
            {
                Property += "arg" + std::to_string(p);
            }
            else
            {
                Property += "arg" + std::to_string(p) + ",";
            }
        }
        generateMetaFiles += "---@return "+ ReturnType + "\n";
        generateMetaFiles += "function " + typeName + "." + MethodName + "(" + Property + ") end\n\n";
    }
}

void GenerateManager::CreateLuaApiJson(const char* outPath)
{
    generateMetaFiles = "---@meta \n";
    int allCount = GetTypeAllCount();
    //모든 타입을 순회
    for (int typeIndex = 0; typeIndex < allCount; typeIndex++)
    {
        PType* targetType = GetTypeByIndex(typeIndex);
        auto metaType = GetTypeCategory(targetType);
        const std::string& targetName = GetTypeName(targetType);

        switch (metaType)
        {
        case META_TYPE::CLASS:
            GenerateLua(GetClass(targetName), typeIndex, targetName);
            break;
        case META_TYPE::NAMESPACE:
            GenerateLua(GetNamespace(targetName), typeIndex, targetName);
            break;
        case META_TYPE::PRIMITIVE:
            break;
        }
    }


    std::ofstream outFile(outPath);
    if (!outFile.is_open()) return;
    outFile << generateMetaFiles;
    outFile.close();
}

std::string GenerateManager::TypeChangeByLua(std::string type)
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
