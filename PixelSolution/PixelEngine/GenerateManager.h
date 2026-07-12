#pragma once
#include "EngineManager.h"
#include "PixelMeta.h"
class LSPBind;
class LuaBind;
class LuaComponentCreate;
class LuaEnumCreate;
class LuaStaticCreate;
class LuaTypeCreate;
class PType;
class GenerateManager :public EngineManager
{
public:
	GenerateManager() = default;
	virtual ~GenerateManager() = default;
	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	void CreateBindCode();
	void LuaGenerate(const char* outPath);
	void LSPGenerate(const char* outPath);
	void JsonGenerate(const char* outPath);
private:
	void AddClassData(PType* type, std::map<std::string, PixelClassMeta>& list);
	void AddClass(PType* type, std::map<std::string, PixelClassMeta>& list);

	std::vector<PixelMemberMeta> TypeMember(PType* type, int memberCount);
	std::vector<PixelMethodMeta> TypeMethod(PType* type, int methodCount);
	std::vector<std::string> TypeEnum(PType* type, int enumCount);
	void CreateRequireFile(const std::string& outPath);

	//LuaBind* luaBind;
	//LSPBind* lspBind;
	LuaComponentCreate* luaComponentCreate = nullptr;
	LuaEnumCreate* luaEnumCreate = nullptr;
	LuaStaticCreate* luaStaticCreate = nullptr;
	LuaTypeCreate* luaTypeCreate = nullptr;

	std::map<std::string, PixelClassMeta> PClassMetaList;
	std::map<std::string, PixelClassMeta> PComponentMetaList;
	std::map<std::string, PixelClassMeta> PTypeMetaList;
	std::map<std::string, PixelEnumMeta> PEnumMetaList;
};

