#include "pch.h"
#include "Scene.h"
#include "PixelEngine.h"
#include "LuaManager.h"
#include "FunctionManager.h"
#include "Core/GameObject.h"
#include "sol.hpp"
#include "SerializeHelper.h"
#include "SPointer.h"
#include "Log.h"
extern PixelEngine* Engine;
Scene::Scene()
{
    sceneName = "";
    path = "";
    lua = nullptr;
    func = nullptr;
    ObjectList = std::unordered_map<size_t, SPointer<GameObject>>();
}
Scene::~Scene(){}

void Scene::Initialize(const std::string& luaPath, const std::string& name)
{
    sceneName = name;
    path = luaPath;
    lua  = Engine->GetFactory<LuaManager>();
    func = Engine->GetFactory<FunctionManager>();
    auto state =  lua->GetModuleCall_Lua();
    sol::protected_function_result result = state->do_file(path);
    if (result.valid())
    {
        table = result;
        table["this"] = this;
        OnStartFunc     = table["Start"];
        OnReleaseFunc   = table["Release"];
    }
    else 
    {
        Log::Error("Not Find SceneFile:" + path);
    }
}


void Scene::Start()
{
    if (OnStartFunc.valid())
    {
        OnStartFunc(table, this);
    }
}

void Scene::Update()
{
    func->FunctionUpdate();
}

void Scene::Release()
{
    if (OnReleaseFunc.valid())
    {
        OnReleaseFunc(table, this);
    }
    func->Clear();
    ObjectList.clear();
}

void Scene::CreateGameObject(SPointer<GameObject> Obj)
{
    ObjectList.insert({ Obj->GetHash(),Obj});
}

std::string Scene::Save(int tab)
{
    std::string content = BeginBlock(tab); // { 시작
    //content += AddEntry(tab + 1, "Name", sceneName);
    //
    //content += BeginBlock(tab + 1, "GameObjects"); // GameObjects = { 시작
    //for (auto& K : ObjectList)
    //{
    //    content += K.second->Save(tab + 2);
    //}
    //content += EndBlock(tab + 1); // GameObjects = } 닫기
    //
    //content += EndBlock(tab, false); // SceneData 전체 } 닫기 (마지막이라 쉼표 생략)
    return content;
}
