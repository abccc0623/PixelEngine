#pragma once
#include "EngineManager.h";
#include <string>
#include <sol/forward.hpp>
class LuaManager : public EngineManager
{
public:
	LuaManager();
	~LuaManager();

	// EngineManager을(를) 통해 상속됨
	void Initialize() override;
	void Update() override;
	void Release() override;
	bool LoadLuaScript(const std::string& fileName);
	sol::state* GetLua();
	bool CreateLuaAPIPath(const std::string& filePath);
private:
	sol::state* lua;
	std::string apiExportPath;
	void LoadDefaultSettingFile();
	void GenerateSimpleStubs(sol::state* state);
};

