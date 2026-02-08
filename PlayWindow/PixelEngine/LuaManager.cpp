#include "LuaManager.h"
#include "LuaWrapper.h"
#include "PixelEngine.h"
#include "GameObject.h"
#include "BaseModule.h"
#include "Module.h"
#include <filesystem>
#include "resource.h"
#include "sol.hpp"
#include <fstream>
#include "ResourceManager.h"
#include "LuaScript.h"
#include "Transform.h"

#define SOL_ALL_SAFETIES_ON 1 // 안전장치 활성화 (권장)

extern std::unordered_map<std::string, std::function<sol::object(sol::state_view, Module*)>> moduleLuaFactories;
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
    lua->open_libraries(sol::lib::base); // 기본 라이브러리 로드

    ///Global함수 등록
    //
    lua->new_enum("Key",
        "W", 87,
        "A", 65,
        "S", 83,
        "D", 68
    );

    //Key관련 등록
    sol::table input = lua->create_named_table("Input");
    input["GetKey"] = GetKey;
    input["GetKeyDown"] = GetKeyDown;
    input["GetKeyUp"] = GetKeyUp;
    input["GetMousePosition_X"] = GetMousePosition_X;
    input["GetMousePosition_Y"] = GetMousePosition_Y;
    //Time관련
    sol::table time = lua->create_named_table("Time");
    time["GetDeltaTime"] = GetDeltaTime;
    time["GetTotalTime"] = GetTotalTime;
    time["GetFPS"] = GetFPS;
    //Engine관련
    sol::table engine = lua->create_named_table("Engine");
    engine["CreateGameObject"] = []() {return Engine->CreateGameObject(); };
    engine["LoadTexture"] = [](std::string path) {return Engine->Load(RESOURCE_TYPE::TEXTURE,path); };

    //생성
    lua->new_usertype<BaseModule>("BaseModule");
    lua->new_usertype<Module>("Module",
        sol::base_classes, sol::bases<BaseModule>());
    lua->new_usertype<LuaScript>("LuaScript",
        sol::base_classes, sol::bases<Module, BaseModule>(),
        "RegisterFile", &LuaScript::RegisterFile
    );
    lua->new_usertype<GameObject>("GameObject",
        "AddModule", [](GameObject& obj, std::string name){obj.AddModule(name);},
        "GetModule", [](GameObject& obj, std::string name, sol::this_state s) -> sol::object
        {
            Module* mod = obj.GetModule(name);
            if (!mod) return sol::nil;
            auto it = moduleLuaFactories.find(name);
            if (it != moduleLuaFactories.end())
            {
                return it->second(s, mod);
            }
            return sol::make_object(s, mod);
        }
    );

    //만약 에셋폴더의 Setting Lua라는 파일이 없을떄 DLL에서 제공하는 파일을 사용함
    LoadDefaultSettingFile();
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
    // 파일 생성 경로 (프로젝트 루트 혹은 API 폴더)
    std::ofstream file(apiExportPath);
    file << "---@meta PixelEngine API\n\n";

    // 자동완성을 뽑고 싶은 Usertype(클래스) 목록
    std::vector<std::string> classes = { "Transform", "GameObject", "LuaScript", "BaseModule", "Module" };
    // 전역 테이블 목록
    std::vector<std::string> globals = { "Input", "Time", "Engine" };

    // 1. 클래스(Usertype) 순회
    for (const auto& className : classes) {
        sol::optional<sol::table> typeTable = (*lua)[className];
        if (!typeTable) continue;

        file << "---@class " << className << "\n";
        file << className << " = {}\n";

        // 클래스의 메타테이블(함수들이 들어있는 곳)을 가져옴
        sol::metatable mt = (*lua)[className];
        if (mt.valid()) {
            mt.for_each([&](sol::object const& key, sol::object const& value) {
                if (key.is<std::string>()) {
                    std::string funcName = key.as<std::string>();
                    // 루아 내부 함수(__index 등)는 제외
                    if (funcName.find("__") != 0 && (value.is<sol::function>() || value.is<sol::protected_function>())) {
                        file << "function " << className << ":" << funcName << "(...) end\n";
                    }
                }
                });
        }
        file << "\n";
    }

    // 2. 전역 테이블(Input, Time 등) 순회
    for (const auto& tableName : globals) {
        sol::optional<sol::table> table = (*lua)[tableName];
        if (!table) continue;

        file << "---@class " << tableName << "\n";
        file << tableName << " = {}\n";

        table->for_each([&](sol::object const& key, sol::object const& value) {
            if (key.is<std::string>()) {
                std::string funcName = key.as<std::string>();
                if (value.is<sol::function>() || value.is<sol::protected_function>()) {
                    file << "function " << tableName << "." << funcName << "(...) end\n";
                }
            }
            });
        file << "\n";
    }

    file.close();
}


void LuaManager::Update()
{

}

void LuaManager::Release()
{

}
