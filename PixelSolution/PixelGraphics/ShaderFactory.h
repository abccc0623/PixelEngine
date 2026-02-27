#pragma once
#include "ResourceFactory.h"
#include <unordered_map>
class ShaderLoader;
class DefaultShaderLoader;
struct ShaderResources;
class ShaderFactory : public ResourceFactory
{
public:
	ShaderFactory();
	~ShaderFactory();

	void Initialize() override;
	void Release() override;
protected:
private:
	ShaderLoader* shaderLoader;
	DefaultShaderLoader* defaultShaderLoader;


	std::unordered_map<Handle16, ShaderResources*> shaderMap;
	std::unordered_map<std::string, ShaderResources*> defaultshaderMap;
	static Handle16 StaticShaderKey;

	// ResourceFactory을(를) 통해 상속됨
	void* GetResource(std::string name) override;

	//friend BufferFactory;
};

