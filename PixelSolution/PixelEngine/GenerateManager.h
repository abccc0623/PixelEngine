#pragma once
#include "EngineManager.h"
#include "PixelMeta.h"
class LSPBind;
class LuaBind;
class LuaClassCreate;
class LuaStaticCreate;
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
	void TypeMember(PixelClassMeta& PixelClass, PType* type);
	void TypeMethod(PixelClassMeta& PixelClass, PType* type);
	void TypeEnum(PixelClassMeta& PixelClass, PType* type);

	LuaBind* luaBind;
	LSPBind* lspBind;
	LuaClassCreate* luaClassCreate;
	LuaStaticCreate* luaStaticCreate;
	std::vector<PixelClassMeta> types;
};

