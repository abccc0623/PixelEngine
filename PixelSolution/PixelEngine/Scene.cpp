#include "pch.h"
#include "Scene.h"
#include "PixelEngine.h"
#include "LuaManager.h"
#include "FunctionManager.h"
#include "Core/GameObject.h"
#include "sol.hpp"
#include "SPointer.h"
#include "WPointer.h"
#include "Log.h"
#include "Export/PixelEngineAPI.h"
#include "LuaSceneInfo.h"
#include "EntityArray.h"
#include "Registry.h"
extern PixelEngine* Engine;
extern SceneChangeCallbackFunc g_SceneObjectChangeCallBack;
Scene::Scene()
{
    sceneName = "";
    path = "";
    lua = nullptr;
    func = nullptr;
    info = nullptr;
    ObjectList = std::unordered_map<size_t, SPointer<GameObject>>();
}
Scene::~Scene(){}
void Scene::Initialize(const std::string& luaPath, const std::string& name)
{
    sceneName = name;
    path = luaPath;
    lua  = Engine->GetFactory<LuaManager>();
    func = Engine->GetFactory<FunctionManager>();
    info = lua->GetSceneLua(name);

    entityArray = new ECS::EntityArray();
    registry = new ECS::Registry();
}


void Scene::Start()
{
    if (info != nullptr)
    {
        info->Start();
    }
}

void Scene::Update()
{
    if (info != nullptr)
    {
        info->Update();
    }
    entityArray->Update();
}

void Scene::Release()
{
    if (info != nullptr)
    {
        info->Release();
    }
    delete entityArray;
    delete registry;
    ObjectList.clear();
}

uint32_t Scene::CreateEntity(const std::string& scriptName)
{
   return entityArray->Create(scriptName);
}

void Scene::DestroyEntity(uint32_t id)
{
    ECS::EntityID entityID(id);
    entityArray->Destroy(entityID);
}

void Scene::CreateGameObject(SPointer<GameObject> Obj)
{
    ObjectList.insert({ Obj->GetHash(),Obj});
    PixelLog::Info("[" + sceneName + "] CreateObject :" + Obj->name);
    if (g_SceneObjectChangeCallBack != nullptr)
    {
        g_SceneObjectChangeCallBack();
    }
}

void Scene::DeleteGameObject(size_t targetObject)
{
    auto find = ObjectList.find(targetObject);
    if (find != ObjectList.end())
    {
        PixelLog::Info("[" + sceneName + "] DeleteObject :" + find->second->name);
        ObjectList.erase(targetObject);
    }
    if (g_SceneObjectChangeCallBack != nullptr) 
    {
        g_SceneObjectChangeCallBack();
    }
}

ECS::Registry* Scene::GetRegistry()
{
    return registry;
}

GameObject** Scene::GetAllSceneObjects(int* maxCount)
{
    *maxCount = static_cast<int>(ObjectList.size());
    Getter.clear();
    Getter.reserve(*maxCount);

    for (auto const& [key, sptr] : ObjectList)
    {
        Getter.push_back(sptr.GetPtr());
    }
    return Getter.empty() ? nullptr : Getter.data();
}

GameObject* Scene::FindGameObject(const std::string& name)
{
    for (auto const& [key, sptr] : ObjectList)
    {
        if (sptr->name == name)
        {
            return sptr.GetPtr();
        }
    }
    return nullptr;
}

WPointer<GameObject> Scene::FindGameObjectToEngine(const std::string& name)
{
    for (auto const& [key, sptr] : ObjectList)
    {
        if (sptr->name == name)
        {
            return WPointer<GameObject>(sptr);
        }
    }
}

WPointer<GameObject> Scene::FindGameObjectToEngine(GameObject* target)
{
    for (auto const& [key, sptr] : ObjectList)
    {
        if (sptr == target)
        {
            return WPointer<GameObject>(sptr);
        }
    }
}
