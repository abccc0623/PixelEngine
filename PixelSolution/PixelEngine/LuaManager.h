#pragma once
#include "EngineManager.h";
#include <string>
#include <vector>
#include <unordered_map>
#include <sol/forward.hpp>
class BindManager;
class KeyInputManager;
class LuaModuleInfo;
class LuaSceneInfo;
class LuaManager : public EngineManager
{
public:
	LuaManager();
	~LuaManager();

	// EngineManager을(를) 통해 상속됨
	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;
	LuaModuleInfo* GetModuleLua(const std::string& fileName);
	LuaSceneInfo* GetSceneLua(const std::string& fileName);

	void ImportLua(const std::string& filePath, const std::string filename, const std::string& ext);
	void CreateLuaManager();
	void ReadEngineGenerateFile();
	void AddEntityID(unsigned int id, sol::table target);
	void RemoveEntityID(unsigned int id);

private:
	sol::state lua;
	sol::table luaManager;

	sol::protected_function AddFunction;
	sol::protected_function RemoveFunction;
	sol::protected_function UpdateFunction;

	std::string SettingKeyEnum();
private:
	BindManager* bind = nullptr;
	KeyInputManager* input = nullptr;

	std::unordered_map<std::string, LuaModuleInfo*> luaModuleTableMap;
	std::unordered_map<std::string, LuaSceneInfo*> luaSceneTableMap;
};

