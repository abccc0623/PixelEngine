#include "Scene.h"
#include "PixelEngine.h"
#include "LuaManager.h"
#include "FunctionManager.h"
#include "GameObject.h"
#include "sol.hpp"
#include <iostream>
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
        std::cout << "Not Find SceneFile:" + path << std::endl;
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

void Scene::CreateGameObject(SPointer<GameObject>& Obj)
{
    ObjectList.insert({ Obj->GetHashCode(),Obj});
}
