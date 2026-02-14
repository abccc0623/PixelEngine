#pragma once
#include <string>
#include <d3d11.h>
#include <unordered_map>
struct ID3D11ShaderReflection;
struct ID3D11InputLayout;
struct ID3D10Blob;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ShaderResources;
struct BufferResources;
class BufferFactory;
#define PIXEL_VS 0
#define PIXEL_PS 1

class ShaderLoader
{
public:
	ShaderLoader();
	virtual ~ShaderLoader();

	virtual void Initialize();
	virtual void Release();
	ShaderResources* CompileShader(std::string ShaderName);
protected:
	virtual ID3D10Blob* LoadCompileShader(int type, std::string name);
private:
	HRESULT CreatePixelShader(ID3D10Blob* mShaderBuffer, ShaderResources* SR);
	HRESULT CreateVertexShader(ID3D10Blob* mShaderBuffer, ShaderResources* SR);
	ID3D11InputLayout* CreateInputLayout(ID3D11ShaderReflection* mReflection, ID3D10Blob* mShaderBuffer);
	ID3D11Buffer* CreateContantBuffer(ID3D11ShaderReflection* mReflection, ShaderResources* SR);
	ID3D11InputLayout* CreateInputLayout(ID3D11ShaderReflection* mReflection, void* mShaderBuffer, SIZE_T);
	DXGI_FORMAT FindFormat(int Number, byte Mask);
	ID3D11SamplerState* CreateSampler();
	static std::unordered_map<std::string, BufferResources*> mContextBufferMap;

	friend BufferFactory;
};

