#include "pch.h"
#include "GenerateManager.h"
#include "PixelMetaAPI.h"
#include "LuaGenerateTemplate.h"
void GenerateManager::Initialize()
{

}

void GenerateManager::Update()
{

}

void GenerateManager::ReleaseShared()
{

}

void GenerateManager::CreateLuaBindCode(const char* outPath)
{
    std::string main = "";
    std::string FunctionName = "inline void BindAll_GeneratedLuaModules(sol::state& lua) \n";
    FunctionName += "{ \n";
    std::string ModuleListFunction = "inline void BindAll_AddModules() \n";
    FunctionName += "\tBindAll_AddModules();\n";
    ModuleListFunction += "{ \n";

    //인클루드
    main += ChangeString(INCLUDE_PATH, "sol.hpp", IncludeString);
    main += ChangeString(INCLUDE_PATH, "Core/Module.h", IncludeString);
    main += ChangeString(INCLUDE_PATH, "Core/GameObject.h", IncludeString);

    main += ChangeString(INCLUDE_PATH, "Module/Transform.h", IncludeString);
    main += ChangeString(INCLUDE_PATH, "Module/Renderer2D.h", IncludeString);
    main += ChangeString(INCLUDE_PATH, "Module/LuaScript.h", IncludeString);
    main += ChangeString(INCLUDE_PATH, "Module/DebugCamera.h", IncludeString);
    main += ChangeString(INCLUDE_PATH, "Module/Camera.h", IncludeString);
    main += "#include <unordered_map>\n";
    main += "extern std::unordered_map <std::string, std::function<sol::object(sol::this_state s, Module* target)>> AddModuleList;\n";

    main += "#define SOL_ALL_SAFETIES_ON 1 \n";

    int classCount = AllClassCount();
     for (int C = 0; C < classCount; C++)
    {
        PClass* pclass = GetClassByIndex(C);
        if (pclass == nullptr) continue;

        std::string typeName = GetClassTypeName(pclass);
        uint64_t hash = GetClassHash(pclass);
        int memberCount = GetClassMemberCount(pclass);
        int methodCount = GetClassMethodCount(pclass);

        main += ChangeString(CLASS_NAME, typeName, LuaFunctionString);
        main += "{ \n";
        main += ChangeString(CLASS_NAME, typeName, LuaClassBindString);
        FunctionName += ChangeString(CLASS_NAME, typeName, LuaCallBindString);

        if (GetClassParentHash(pclass) == GetClassHashByString("Module")) 
        {
            std::unordered_map<std::string, std::string> replaceData =
            {
                {"{{CLASS_NAME}}", typeName},
            };
            ModuleListFunction += ChangeString(replaceData,LuaAddModuleString);
        }

        //맴버 등록
        for (int m = 0; m < memberCount; m++)
        {
            std::string memberName = GetClassMemberName(pclass, m);
            std::string memberType = GetClassMemberType(pclass, m);
            std::unordered_map<std::string, std::string> replaceData =
            {
                {"{{CLASS_NAME}}", typeName},
                {"{{MEMBER_NAME}}", memberName}
            };
            main += ChangeString(replaceData, LuaMemberBindString);
        }
        //함수 등록 
        for (int m = 0; m < methodCount; m++)
        {
            std::string methodName = GetClassMethodName(pclass, m);

            if (HasClassMethodFlag(pclass, m, MetaFlag::LUABIND) == false) continue;

            std::unordered_map<std::string, std::string> replaceData =
            {
                {"{{CLASS_NAME}}", typeName},
                {"{{METHOD_NAME}}", methodName}
            };
            main += ChangeString(replaceData, LuaMethodBindString);
        }
        main += "} \n\n";
    }
    FunctionName += "} \n";
    ModuleListFunction += "} \n";
    main += ModuleListFunction;
    main += FunctionName;

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
