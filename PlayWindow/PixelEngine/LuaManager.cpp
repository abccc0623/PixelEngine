#include "LuaManager.h"
#include "LuaWrapper.h"
#include "PixelEngine.h"
#include "GameObject.h"
#include "BaseModule.h"
#include "Module.h"
#include "resource.h"
#include "sol.hpp"
#include <filesystem>
#include <fstream>
#include "ResourceManager.h"
#include "LuaScript.h"
#include "Transform.h"
#include "BindManager.h"
#define SOL_ALL_SAFETIES_ON 1 // 안전장치 활성화 (권장)

//extern std::unordered_map<std::string, std::function<sol::object(sol::state_view, Module*)>> moduleLuaFactories;
extern PixelEngine* Engine;
LuaManager::LuaManager()
{

}

LuaManager::~LuaManager()
{
    delete lua;
}
void LuaManager::Initialize()
{
    lua = new sol::state();
    lua->open_libraries(
        sol::lib::base,    // print, assert, type 등 기본 함수
        sol::lib::package, // require, package.path 등 (첫 번째 에러 해결)
        sol::lib::table,   // table.insert, table.remove 등 (두 번째 에러 해결)
        sol::lib::string,  // 문자열 처리
        sol::lib::math,    // 수학 연산
        sol::lib::os,      // 시간(os.time) 등 시스템 함수
        sol::lib::debug    // 디버깅 툴
    );
    ///Global함수 등록
    //
    //lua->new_enum("Key",
    //    "W", 87,
    //    "A", 65,
    //    "S", 83,
    //    "D", 68
    //);

    bind = Engine->GetFactory<BindManager>();
   

    //Key관련 등록
    sol::table input = lua->create_named_table("Input");
    input["GetKey"] = GetKey;
    input["GetKeyDown"] = GetKeyDown;
    input["GetKeyUp"] = GetKeyUp;
    input["GetMousePosition_X"] = GetMousePosition_X;
    input["GetMousePosition_Y"] = GetMousePosition_Y;
    std::vector<std::string> functionName = std::vector<std::string>();
    functionName.push_back("function Input.GetKey(...)");
    functionName.push_back("function Input.GetKeyDown(...)");
    functionName.push_back("function Input.GetKeyUp(...)");
    functionName.push_back("function Input.GetMousePosition_X(...)");
    functionName.push_back("function Input.GetMousePosition_Y(...)");
    AddLuaAPI("Input", functionName);

    //Time관련
    sol::table time = lua->create_named_table("Time");
    time["GetDeltaTime"] = GetDeltaTime;
    time["GetTotalTime"] = GetTotalTime;
    time["GetFPS"] = GetFPS;
    functionName.clear();
    functionName.push_back("function Time.GetDeltaTime(...)");
    functionName.push_back("function Time.GetTotalTime(...)");
    functionName.push_back("function Time.GetFPS(...)");
    AddLuaAPI("Time", functionName);


    //Engine관련
    sol::table engine = lua->create_named_table("Engine");
    engine["CreateGameObject"] = []() {return Engine->CreateGameObject(); };
    engine["LoadTexture"] = [](std::string path) {return LoadTexture(path);};
    functionName.clear();
    functionName.push_back("function Engine.CreateGameObject(...)");
    functionName.push_back("function Engine.LoadTexture(...)");
    AddLuaAPI("Engine", functionName);


    //생성
    lua->new_usertype<BaseModule>("BaseModule");
    lua->new_usertype<Module>("Module",
        sol::base_classes, sol::bases<BaseModule>());
    lua->new_usertype<LuaScript>("LuaScript",
        sol::base_classes, sol::bases<Module, BaseModule>(),
        "RegisterFile", &LuaScript::RegisterFile
    );

    //게임 오브젝트 추가
    lua->new_usertype<GameObject>("GameObject",
        "AddModule", [](GameObject& obj, std::string name, sol::this_state s) ->sol::object
        {
            auto bind = Engine->GetFactory<BindManager>();
            obj.AddModule(name);
            return bind->GetLua(s, obj, name);
        },
        "GetModule", [](GameObject& obj, std::string name, sol::this_state s) -> sol::object
        {
            auto bind = Engine->GetFactory<BindManager>();
            return bind->GetLua(s, obj,name);
        }
    );
    functionName.clear();
    functionName.push_back("function GameObject.AddModule(...)");
    functionName.push_back("function GameObject.GetModule(...)");
    AddLuaAPI("GameObject", functionName);
}


bool LuaManager::LoadLuaScript(const std::string& fileName)
{
    if (!std::filesystem::exists(fileName)) {
        std::cerr << "Error: Lua file not found at path: " << fileName << std::endl;
        return false;
    }

    auto result = lua->script_file(fileName);
    if (!result.valid()) {
        sol::error err = result;
        std::string what = err.what();
        std::cout << "루아 로드 에러: " << what << std::endl;
    }
	return true;
}

sol::state* LuaManager::GetLua()
{
    if (lua != nullptr){return lua;}
    return nullptr;
}

void LuaManager::AddLuaAPI(std::string className, std::vector<std::string> functionName)
{
    apiDefinitions += "\n";
    apiDefinitions += "---@class " + className + "\n";
    apiDefinitions += className + "= {} \n";
    for (int i = 0; i < functionName.size(); i++)
    {
        apiDefinitions += functionName[i];
        apiDefinitions += " end \n";
    }
    apiDefinitions += "\n";
}

bool LuaManager::CreateLuaAPIPath(const std::string& filePath)
{
    apiExportPath = filePath;
    GenerateSimpleStubs(lua);
    return true;
}

void LuaManager::LoadDefaultSettingFile()
{
    //만약 에셋폴더의 Setting Lua라는 파일이 없을떄 DLL에서 제공하는 파일을 사용함
    if (!std::filesystem::exists("./Asset/Setting.lua"))
    {
        std::cerr << "Start Engine DefaultSetting File" << std::endl;

        HMODULE hModule = GetModuleHandle(L"PixelEngine.dll");
        HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(IDR_LUA1), L"LUA");

        HGLOBAL hData = LoadResource(hModule, hRes);
        DWORD size = SizeofResource(hModule, hRes);
        const char* pData = (const char*)LockResource(hData);

        if (pData && size > 0)
        {
            // pData부터 size만큼만 복사해서 안전한 C++ 문자열 생성
            std::string safeScript(pData, size);

            // 에러를 상세히 보기 위해 보호된 실행(safe_script) 권장
            auto result = lua->safe_script(safeScript, sol::script_pass_on_error);
            if (!result.valid()) 
            {
                sol::error err = result;
                std::string what = err.what();
                std::cout << "루아 리소스 실행 에러: " << what << std::endl;
            }
            else 
            {
                sol::table testTable = result;
            }
        }
    }
}
void LuaManager::GenerateSimpleStubs(sol::state* lua)
{
    std::filesystem::path p(apiExportPath);
    if (p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    std::ofstream file(apiExportPath, std::ios::out | std::ios::trunc);
    if (file.is_open()) {
        file << "---@meta PixelEngine API\n\n";
        file << apiDefinitions;
        file << "\n";
        file.close();
    }
    else {
        // 경로가 잘못되었거나 파일이 사용 중일 때에 대한 예외 처리
        std::cerr << "Failed to open file: " << apiExportPath << std::endl;
    }
}


void LuaManager::Update()
{

}

void LuaManager::Release()
{

}
