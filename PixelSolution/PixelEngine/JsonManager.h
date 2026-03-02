#pragma once
#include "EngineManager.h"
#include <json_fwd.hpp>
class JsonManager : public EngineManager
{
public:
	JsonManager();
	virtual ~JsonManager();

	void Initialize() override;
	void Update() override;
	void ReleaseShared() override;

	static void LoadGameObject(nlohmann::ordered_json file);
	static void LoadScene(nlohmann::ordered_json file);
	

	static bool Load(const std::string& path);
	static bool Save(const std::string& path, const nlohmann::ordered_json& j);
};

