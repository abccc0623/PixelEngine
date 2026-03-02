#include "pch.h"
#include "JsonManager.h"
#include "json.hpp"
#include <fstream>
#include "PixelEngineAPI.h"
#include "Export/Core/GameObject.h"
#include "Export/Core/Module.h"
#include "Export/Type/PMeta.h"
#include "Export/Module/Transform.h"


JsonManager::JsonManager()
{

}

JsonManager::~JsonManager()
{

}

bool JsonManager::Load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) return false;
    nlohmann::ordered_json LoadFile = nlohmann::ordered_json::parse(file);
    std::string fileType = LoadFile["FileType"];
        
    if (fileType == "SceneFile")
    {
        LoadScene(LoadFile);
    }

    return true;
}

bool JsonManager::Save(const std::string& path, const nlohmann::ordered_json& j)
{
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << j.dump(4);
    return true;
}

void JsonManager::Initialize()
{
    

}

void JsonManager::Update()
{
}

void JsonManager::ReleaseShared()
{

}

void JsonManager::LoadScene(nlohmann::ordered_json file)
{
    std::string name = file["SceneName"];
    CreateScene(name.c_str());
    ChangeScene(name.c_str());

    //게임오브젝트 호출
    for (auto& mod : file["GameObjects"])
    {
        LoadGameObject(mod);
    }
}
void JsonManager::LoadGameObject(nlohmann::ordered_json file)
{
    std::string ObjectName = file["Name"];
    unsigned long ObjectHash = file["Hash"];
    GameObject* target = CreateGameObject(ObjectName.c_str());

    for (auto& mod : file["Modules"])
    {
        int TypeNumber = mod["Type"];
        target->AddModule(MODULE_TYPE(TypeNumber));
        auto k = target->GetModule(MODULE_TYPE(TypeNumber));

        if (TypeNumber == (int)MODULE_TYPE::Transform)
        {
            auto m = static_cast<Transform*>(k);
            m->meta.Position.X = mod["Position"][0].get<float>();
            m->meta.Position.Y = mod["Position"][1].get<float>();
            m->meta.Position.Z = mod["Position"][2].get<float>();
            m->meta.Rotation.X = mod["Rotation"][0].get<float>();
            m->meta.Rotation.Y = mod["Rotation"][1].get<float>();
            m->meta.Rotation.Z = mod["Rotation"][2].get<float>();
            m->meta.Scale.X = mod["Scale"][0].get<float>();
            m->meta.Scale.Y = mod["Scale"][1].get<float>();
            m->meta.Scale.Z = mod["Scale"][2].get<float>();
        }
    }
}


