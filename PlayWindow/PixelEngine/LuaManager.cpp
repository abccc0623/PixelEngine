#include "LuaManager.h"
#include "LuaWrapper.h"
#include "PixelEngine.h"
#include "GameObject.h"
#define SOL_ALL_SAFETIES_ON 1 // 안전장치 활성화 (권장)

extern PixelEngine* Engine;
LuaManager::LuaManager()
{

}

LuaManager::~LuaManager()
{

}
void LuaManager::Initialize()
{
    lua.open_libraries(sol::lib::base); // 기본 라이브러리 로드

    ///Global함수 등록
    //Key관련 등록
    sol::table input = lua.create_named_table("Input");
    input["GetKey"] = GetKey;
    input["GetKeyDown"] = GetKeyDown;
    input["GetKeyUp"] = GetKeyUp;
    input["GetMousePosition_X"] = GetMousePosition_X;
    input["GetMousePosition_Y"] = GetMousePosition_Y;
    //Time관련
    sol::table time = lua.create_named_table("Time");
    time["GetDeltaTime"] = GetDeltaTime;
    time["GetTotalTime"] = GetTotalTime;
    time["GetFPS"] = GetFPS;

    //생성








    lua.new_usertype<GameObject>("GameObject",
        // "루아에서 쓸 이름", &실제함수주소 또는 람다
        "AddModule", [](GameObject& obj, std::string name){obj.AddModule(name);}
    );

    sol::table engine = lua.create_named_table("Engine");
    engine["CreateGameObject"] = []() {return Engine->CreateGameObject(); };

}


bool LuaManager::LoadLuaScript(const std::string& fileName)
{
    auto result = lua.script_file("test.lua");
    if (!result.valid()) {
        sol::error err = result;
        std::string what = err.what();
        std::cout << "루아 로드 에러: " << what << std::endl;
    }
	return true;
}




void LuaManager::Update()
{

}

void LuaManager::Release()
{

}
