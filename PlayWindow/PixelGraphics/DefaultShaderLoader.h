#pragma once
#include "ShaderLoader.h"
class DefaultShaderLoader :public ShaderLoader
{
public:
	DefaultShaderLoader();
	~DefaultShaderLoader();

	void Initialize() override;
	void Release() override;
protected:
	ID3D10Blob* LoadCompileShader(int type, std::string name) override;
private:
	void* DefaultLoadShader(const wchar_t* resourceName, DWORD& outSize);
};

