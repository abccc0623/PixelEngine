#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <d3dcompiler.h>

#include "PixelResources.h"
#include "ResourceFactory.h"

namespace PixelGraphics
{
	class GraphicsCore;

	class ShaderFactory : public ResourceFactory
	{
	public:
		bool Initialize(GraphicsCore* graphicsCore) override;
		void Release() override;
		void Clear() override;
		std::uint16_t Load(const std::string& path) override;

		ShaderResources* Get(std::uint16_t key);

	private:
		bool LoadDefaultShader(std::uint16_t key, const wchar_t* vertexResourceName, const wchar_t* pixelResourceName);
		bool CompileFile(const std::wstring& path, const char* target, ID3DBlob** shaderBlob);
		bool CompileResource(const wchar_t* resourceName, const char* target, ID3DBlob** shaderBlob);
		bool CreateShaderResources(ID3DBlob* vertexShaderBlob, ID3DBlob* pixelShaderBlob, ShaderResources* shader);
		bool CreateInputLayout(ID3DBlob* vertexShaderBlob, ID3D11InputLayout** inputLayout);
		std::uint16_t AllocateKey();
		void ReleaseShader(ShaderResources* shader);

		GraphicsCore* graphicsCore = nullptr;
		std::uint16_t defaultShaderKey = 0;
		std::uint16_t nextShaderKey = 2;
		std::unordered_map<std::uint16_t, std::unique_ptr<ShaderResources>> shaders;
	};
}
