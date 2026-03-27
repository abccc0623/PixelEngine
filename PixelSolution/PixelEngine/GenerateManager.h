#pragma once
#include "EngineManager.h"
#include "PixelMeta.h"
class LSPBind;
class LuaBind;
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

	void CreateBindCode();
	void LuaGenerate(const char* outPath);
	void LSPGenerate(const char* outPath);
	void JsonGenerate(const char* outPath);
private:
	LuaBind* luaBind;
	LSPBind* lspBind;
	std::vector<PixelClassMeta> types;
};

