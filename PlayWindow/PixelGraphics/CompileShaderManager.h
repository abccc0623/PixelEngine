#pragma once

#include <string>
#include <map>
#include "GraphicsCore.h"
typedef ULONG_PTR SIZE_T, * PSIZE_T;
struct ID3D11ShaderReflection;
struct ID3D11InputLayout;
struct ID3D10Blob;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ShaderResources;

class CompileShaderManager : public GraphicsCore
{
public:
	enum class SHADER_TYPE : int
	{
		PS,
		VS,
	};
public:
	CompileShaderManager();
	virtual ~CompileShaderManager();
public:
	std::string ShaderFilePath = "../Assets/Shader/";
public:
	void Initialize();
	void Release();
	ShaderResources*	DefaultCompileShader(std::string ShaderName);
	ShaderResources*	CompileShader(std::string ShaderName);
	ID3D11SamplerState*	CreateSampler();
private:
	void* headerPtr = nullptr;
	DWORD headerSize = 0;

	void* DefaultLoadShader(const wchar_t* resourceName, DWORD& outSize);

	ID3D10Blob*			DefaultCompileShader(SHADER_TYPE mType, std::string& ShaderName, ShaderResources* SR);
	ID3D10Blob*			CompileShader(SHADER_TYPE mType,std::string& ShaderName);

	//쉐이더파일을 통해 얻을 수 있는 파일들을 생성한다.
	HRESULT				CreateVertexShader(ID3D10Blob* mShaderBuffer, ShaderResources* SR);
	HRESULT				CreatePixelShader(ID3D10Blob* mShaderBuffer,  ShaderResources* SR);
	ID3D11InputLayout*	CreateInputLayout(ID3D11ShaderReflection* mReflection, ID3D10Blob* mShaderBuffer);
	ID3D11Buffer*		CreateContantBuffer(ID3D11ShaderReflection* mReflection, ShaderResources* SR);

	ID3D11InputLayout* CreateInputLayout(ID3D11ShaderReflection* mReflection, void* mShaderBuffer, SIZE_T);
	DXGI_FORMAT FindFormat(int Number, byte Mask);
};

