#pragma once
#include "EngineManager.h"
class PClass;
class PNamespace;
class GenerateManager :public EngineManager
{
public:
	GenerateManager() = default;
	virtual ~GenerateManager() = default;
	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	void Generate(PClass* target,int index,std::string typeName);
	void Generate(PNamespace* target, int index, std::string typeName);


	void CreateLuaBindCode(const char* outPath);
	void CreateLuaApiJson(const char* outPath);
private:
	std::string ChangeString(const std::string& command, const std::string& changeName, const std::string& templateStr);
	std::string ChangeString(const std::unordered_map<std::string, std::string>& replacements, const std::string& templateStr);
	std::string includeSTR;
	std::string contentSTR;
	std::string functionSTR;
	std::string addModuleSTR;
	std::string globalContentSTR;
};

