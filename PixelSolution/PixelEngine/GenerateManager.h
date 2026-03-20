#pragma once
#include "EngineManager.h"
class GenerateManager :public EngineManager
{
public:
	GenerateManager() = default;
	virtual ~GenerateManager() = default;
	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	void CreateLuaBindCode(const char* outPath);

private:
	std::string ChangeString(const std::string& command, const std::string& changeName, const std::string& templateStr);
	std::string ChangeString(const std::unordered_map<std::string, std::string>& replacements, const std::string& templateStr);
};

