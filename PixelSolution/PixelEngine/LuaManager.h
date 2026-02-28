#pragma once
#include "EngineManager.h";
#include <string>
#include <vector>
#include <unordered_map>
#include <sol/forward.hpp>
class BindManager;
class KeyInputManager;
class ObjectManager;
class LuaClassInfo;
class LuaManager : public EngineManager
{
public:
	LuaManager();
	~LuaManager();

	// EngineManager을(를) 통해 상속됨
	void Initialize() override;
	void Update() override;
	void ReleaseShared() override;
	bool CreateLuaAPIPath(const std::string& filePath);
	sol::state* GetModuleCall_Lua();
	void AddLuaAPI(std::string className,std::vector<std::string> functionName);

	bool Load(const std::string& filePath);
	LuaClassInfo* GetLua(const std::string& fileName);
private:
	sol::state* lua;
	std::string apiDefinitions;
	std::string apiExportPath;
	void LoadDefaultSettingFile();
	std::string SettingKeyEnum();
private:
	BindManager* bind = nullptr;
	KeyInputManager* input = nullptr;
	ObjectManager* obj = nullptr;

	std::unordered_map<std::string, LuaClassInfo*> luaTableMap;
};

