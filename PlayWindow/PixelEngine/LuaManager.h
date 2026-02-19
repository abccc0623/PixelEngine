#pragma once
#include "EngineManager.h";
#include <string>
#include <vector>
#include <sol/forward.hpp>
class BindManager;
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
	bool CreateLuaAPIPath(const std::string& filePath);
	sol::state* GetLua();
	void AddLuaAPI(std::string className,std::vector<std::string> functionName);
private:
	sol::state* lua;
	std::string apiDefinitions;
	std::string apiExportPath;
	BindManager* bind;
	void LoadDefaultSettingFile();
	void GenerateSimpleStubs(sol::state* state);
};

