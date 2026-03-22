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
    else if(GetClassHash(target) == GetClassHashByString("PVector3"))
    {
        //Type형태의 클래스일 경우
        includeSTR += ChangeString("{{INCLUDE_TYPE}}", typeName, IncludeTypesString);
    }
    else if (GetClassHash(target) == GetClassHashByString("GameObject"))
    {
        //Core형태의 클래스일 경우
        includeSTR += ChangeString("{{INCLUDE_TYPE}}", typeName, IncludeCoreString);
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

void GenerateManager::CreateLuaApiJson(const char* outPath)
{
    json root;
    root["ApiVersion"] = "1.0";
    root["Types"] = json::array();

    int allCount = GetTypeAllCount();

    // 기존 바인딩 코드와 동일하게 모든 타입을 순회합니다.
    for (int typeIndex = 0; typeIndex < allCount; typeIndex++)
    {
        PType* targetType = GetTypeByIndex(typeIndex);
        auto metaType = GetTypeCategory(targetType);
        const std::string& targetName = GetTypeName(targetType);

        // Primitive 타입(int, float 등)은 자동완성에 안 띄우므로 패스
        if (metaType == META_TYPE::PRIMITIVE) continue;

        json typeJson;
        typeJson["Name"] = targetName;
        typeJson["Fields"] = json::array();
        typeJson["Functions"] = json::array();

        // ==========================================
        // 1. 클래스(CLASS) 파싱
        // ==========================================
        if (metaType == META_TYPE::CLASS)
        {
            typeJson["Kind"] = "Class";
            PClass* target = GetClass(targetName);

            // 1-1. 멤버 변수 추출
            int memberCount = GetClassMemberCount(target);
            for (int m = 0; m < memberCount; m++)
            {
                json fieldJson;
                fieldJson["Name"] = GetClassMemberName(target, m);
                fieldJson["Type"] = GetClassMemberType(target, m);
                typeJson["Fields"].push_back(fieldJson);
            }

            // 1-2. 멤버 함수 추출
            int methodCount = GetClassMethodCount(target);
            for (int m = 0; m < methodCount; m++)
            {
                // LUABIND 플래그가 없으면 자동완성에도 안 띄웁니다!
                if (HasClassMethodFlag(target, m, MetaFlag::LUABIND) == false) continue;

                json funcJson;
                funcJson["Name"] = GetClassMethodName(target, m);

                // [!] 10년 차의 팁: 기존 리플렉션 시스템에 아래와 같이 반환형과 파라미터를 
                // 가져오는 함수가 있다면 여기에 연결해 주세요.
                // funcJson["ReturnType"] = GetClassMethodReturnType(target, m);
                // funcJson["Parameters"] = ... (파라미터 리스트를 배열로 추가)

                typeJson["Functions"].push_back(funcJson);
            }
        }
        // ==========================================
        // 2. 네임스페이스(NAMESPACE) / 전역 함수 파싱
        // ==========================================
        else if (metaType == META_TYPE::NAMESPACE)
        {
            typeJson["Kind"] = "Namespace";
            PNamespace* target = GetNamespace(targetName);

            int namespaceCount = GetNamespaceMethodCount(target);
            for (int m = 0; m < namespaceCount; m++)
            {
                if (HasNamespaceMethodFlag(target, m, MetaFlag::LUABIND) == false) continue;

                json funcJson;
                funcJson["Name"] = GetNamespaceMethodName(target, m);

                // 네임스페이스 함수도 마찬가지로 반환형과 파라미터를 넣어주면 좋습니다.
                // funcJson["ReturnType"] = GetNamespaceMethodReturnType(target, m);

                typeJson["Functions"].push_back(funcJson);
            }
        }
        root["Types"].push_back(typeJson);
    }
    // 파일로 예쁘게(4칸 들여쓰기) 출력
    std::ofstream outFile(outPath);
    if (!outFile.is_open()) return;
    outFile << root.dump(4);
    outFile.close();
}