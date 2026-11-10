#include "pch.h"
#include "SceneManager.h"
#include "SceneObject.h"
#include "Log.h"
#include "PixelEngine.h"
#include "LuaManager.h"
#include "CoroutineManager.h"
#include "PhysManager.h"

extern PixelEngine* Engine;
SceneManager::SceneManager() = default;
SceneManager::~SceneManager() = default;

void SceneManager::Initialize()
{
}

void SceneManager::Update()
{
    if (nowScene != nullptr)
    {
        nowScene->Update();
    }
}

void SceneManager::ReleaseCurrentScene()
{
    if (nowScene == nullptr) return;
    // Run before the shared Lua/physics managers are destroyed, and outside callbacks.
    Engine->GetFactory<CoroutineManager>()->Clear();
    nowScene->Release();
    Engine->GetFactory<LuaManager>()->ResetScene();
    Engine->GetFactory<PhysManager>()->ClearCollisionEvents();
    nowScene.reset();
}

void SceneManager::Release()
{
    pendingScene.clear();
    ReleaseCurrentScene();
    SceneMap.clear();
}

void SceneManager::ChangeScene(std::string name)
{
    if (SceneMap.find(name) == SceneMap.end())
    {
        PixelLog::Error("Not Find Scene :" + name);
        return;
    }
    // A valid later request replaces an earlier request in this frame.
    pendingScene = std::move(name);
}

void SceneManager::ApplyPendingChange()
{
    if (pendingScene.empty()) return;
    std::string name;
    name.swap(pendingScene);
    if (nowScene != nullptr && nowScene->GetSceneName() == name) return;

    auto found = SceneMap.find(name);
    if (found == SceneMap.end()) return;
    const std::string path = found->second;
    ReleaseCurrentScene();
    nowScene = std::make_unique<SceneObject>();
    nowScene->Initialize(path, name);
    nowScene->Start();
    // A request from Initialize/Start is intentionally left for the next frame.
}

SceneObject* SceneManager::GetNowScene()
{
    if (nowScene == nullptr)
    {
        static const std::string defaultSceneName = "NewPixelDefaultScene";
        SceneMap.emplace(defaultSceneName, "");
        nowScene = std::make_unique<SceneObject>();
        nowScene->Initialize("", defaultSceneName);
        nowScene->Start();
    }
    return nowScene.get();
}

void SceneManager::Clear()
{
    Release();
}

uint32_t SceneManager::CreateEntity(const std::string& scriptName)
{
    return GetNowScene()->CreateEntity(scriptName);
}

ECS::EntityObject* SceneManager::FindEntity(uint32_t id)
{
    return GetNowScene()->FindEntity(id);
}

void SceneManager::ActiveEntity(uint32_t id, bool active)
{
    GetNowScene()->ActiveEntity(id, active);
}

void SceneManager::DestroyEntity(uint32_t id)
{
    GetNowScene()->DestroyEntity(id);
}

ECS::Registry* SceneManager::GetRegistry()
{
    return GetNowScene()->GetRegistry();
}

void SceneManager::CreateScene(const std::string& luaPath)
{
    const std::filesystem::path path(luaPath);
    const std::string name = path.stem().string();
    if (!SceneMap.emplace(name, luaPath).second)
    {
        PixelLog::Error("This scene name is already in use :" + name);
    }
}
