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
	bool Load(const std::string& filePath);
	LuaClassInfo* GetLua(const std::string& fileName);
	
	std::string ChangeLuaType(std::string type);


	void BindEngine();
	void BindLuaKey();
	void BindLuaTime();
	void BindLuaSetting();
	void BindAsset();
private:
	sol::state lua;
	std::string SettingKeyEnum();
private:
	BindManager* bind = nullptr;
	KeyInputManager* input = nullptr;
	ObjectManager* obj = nullptr;

	std::unordered_map<std::string, LuaClassInfo*> luaTableMap;
};

