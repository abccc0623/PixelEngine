/** 구조체 관련 */
#include "ShaderFactory.h"
#include "PixelResources.h"
#include "PixelGraphicsLib.h"
#include "ShaderLoader.h"
#include "DefaultShaderLoader.h"
#include "KeyAllocator.h"
#include <iostream>

Handle16 ShaderFactory::StaticShaderKey = 0;

std::unordered_map<std::string, BufferResources*> ShaderLoader::mContextBufferMap;
ShaderFactory::ShaderFactory()
{
	shaderLoader = nullptr;
	defaultShaderLoader = nullptr;
	shaderMap = std::unordered_map<Handle16, ShaderResources*>();
	defaultshaderMap = std::unordered_map<std::string, ShaderResources*>();
}

ShaderFactory::~ShaderFactory()
{

}

void ShaderFactory::Initialize()
{
	shaderLoader = new ShaderLoader();
	defaultShaderLoader = new DefaultShaderLoader();

	shaderLoader->Initialize();
	defaultShaderLoader->Initialize();

	//기본 스테틱 쉐이더
	auto static_Shader = defaultShaderLoader->CompileShader("STATIC");
	static_Shader->key = keyAllocator.GetKey16();
	defaultshaderMap.insert({ "Static" ,static_Shader});
}


void ShaderFactory::Release()
{
	shaderLoader->Release();
	defaultShaderLoader->Release();

	for (auto& K : shaderMap)
	{
		K.second->mLayout->Release();
		K.second->mPixelShader->Release();
		K.second->mVertexShader->Release();
	}
	shaderMap.clear();
	delete shaderLoader;
	delete defaultShaderLoader;
}

void* ShaderFactory::GetResource(std::string name)
{
	auto k = defaultshaderMap.find(name.c_str());
	if (k == defaultshaderMap.end())
	{
		std::cout<< "Not Find Shader :" + name <<std::endl;
		return nullptr;
	}
	else
	{
		return k->second;
	}
}


