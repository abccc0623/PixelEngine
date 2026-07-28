#include "pch.h"
#include "ShaderFactory.h"

#include <d3d11shader.h>
#include <d3dcompiler.h>

#include "GraphicsCore.h"
#include "ResourceManager.h"

namespace
{
	DXGI_FORMAT GetInputFormat(D3D_REGISTER_COMPONENT_TYPE componentType, BYTE mask)
	{
		if (mask == 1)
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_UINT32:
				return DXGI_FORMAT_R32_UINT;
			case D3D_REGISTER_COMPONENT_SINT32:
				return DXGI_FORMAT_R32_SINT;
			default:
				return DXGI_FORMAT_R32_FLOAT;
			}
		}
		if (mask <= 3)
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_UINT32:
				return DXGI_FORMAT_R32G32_UINT;
			case D3D_REGISTER_COMPONENT_SINT32:
				return DXGI_FORMAT_R32G32_SINT;
			default:
				return DXGI_FORMAT_R32G32_FLOAT;
			}
		}
		if (mask <= 7)
		{
			switch (componentType)
			{
			case D3D_REGISTER_COMPONENT_UINT32:
				return DXGI_FORMAT_R32G32B32_UINT;
			case D3D_REGISTER_COMPONENT_SINT32:
				return DXGI_FORMAT_R32G32B32_SINT;
			default:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			}
		}

		switch (componentType)
		{
		case D3D_REGISTER_COMPONENT_UINT32:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		case D3D_REGISTER_COMPONENT_SINT32:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		default:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
	}

	void PrintShaderError(ID3DBlob* errorBlob)
	{
		if (errorBlob)
		{
			OutputDebugStringA(
				static_cast<const char*>(
					errorBlob->GetBufferPointer()));
		}
	}
}

bool PixelGraphics::ShaderFactory::Initialize(GraphicsCore* core)
{
	Clear();
	graphicsCore = core;
	defaultShaderKey = static_cast<std::uint16_t>(ResourceDefaultKey::SHADER_STATIC);
	nextShaderKey = 2;

	const bool staticLoaded = LoadDefaultShader(
		static_cast<std::uint16_t>(ResourceDefaultKey::SHADER_STATIC),
		L"STATIC_VS",
		L"STATIC_PS");
	const bool debugLoaded = LoadDefaultShader(
		static_cast<std::uint16_t>(ResourceDefaultKey::SHADER_DEBUG),
		L"DEBUG_VS",
		L"DEBUG_PS");

	return staticLoaded && debugLoaded;
}

void PixelGraphics::ShaderFactory::Release()
{
	Clear();
	graphicsCore = nullptr;
}

void PixelGraphics::ShaderFactory::Clear()
{
	for (auto& [key, shader] : shaders)
	{
		ReleaseShader(shader.get());
	}

	shaders.clear();
	constantBuffers.clear();
	nextShaderKey = 2;
}

std::uint16_t PixelGraphics::ShaderFactory::AllocateKey()
{
	constexpr std::uint16_t invalidKey =
		(std::numeric_limits<std::uint16_t>::max)();

	for (std::uint32_t count = 0; count < invalidKey - 2; ++count)
	{
		if (nextShaderKey < 2 || nextShaderKey == invalidKey)
		{
			nextShaderKey = 2;
		}

		const std::uint16_t key = nextShaderKey++;
		if (shaders.find(key) == shaders.end())
		{
			return key;
		}
	}

	return invalidKey;
}

std::uint16_t PixelGraphics::ShaderFactory::Load(const std::string& path)
{
	for (const auto& [key, shader] : shaders)
	{
		if (shader && shader->path == path)
		{
			return key;
		}
	}

	if (!graphicsCore || !graphicsCore->GetDevice())
	{
		return defaultShaderKey;
	}

	const std::wstring basePath(path.begin(), path.end());
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;

	if (!CompileFile(
		basePath + L"_vs.hlsl",
		"vs_5_0",
		vertexShaderBlob.GetAddressOf()) ||
		!CompileFile(
			basePath + L"_ps.hlsl",
			"ps_5_0",
			pixelShaderBlob.GetAddressOf()))
	{
		return defaultShaderKey;
	}

	auto shader = std::make_unique<ShaderResources>();
	if (!CreateShaderResources(
		vertexShaderBlob.Get(),
		pixelShaderBlob.Get(),
		shader.get()))
	{
		ReleaseShader(shader.get());
		return defaultShaderKey;
	}

	const std::uint16_t key = AllocateKey();
	constexpr std::uint16_t invalidKey =
		(std::numeric_limits<std::uint16_t>::max)();
	if (key == invalidKey)
	{
		ReleaseShader(shader.get());
		return defaultShaderKey;
	}

	shader->key = key;
	shader->path = path;
	shaders.emplace(key, std::move(shader));
	return key;
}

bool PixelGraphics::ShaderFactory::LoadDefaultShader(std::uint16_t key, const wchar_t* vertexResourceName, const wchar_t* pixelResourceName)
{
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;

	if (!CompileResource(
		vertexResourceName,
		"vs_5_0",
		vertexShaderBlob.GetAddressOf()) ||
		!CompileResource(
			pixelResourceName,
			"ps_5_0",
			pixelShaderBlob.GetAddressOf()))
	{
		return false;
	}

	auto shader = std::make_unique<ShaderResources>();
	if (!CreateShaderResources(
		vertexShaderBlob.Get(),
		pixelShaderBlob.Get(),
		shader.get()))
	{
		ReleaseShader(shader.get());
		return false;
	}

	shader->key = key;
	shader->path =
		key == defaultShaderKey
		? "DEFAULT_STATIC_SHADER"
		: "DEFAULT_DEBUG_SHADER";
	shaders.emplace(key, std::move(shader));
	return true;
}

bool PixelGraphics::ShaderFactory::CompileFile(const std::wstring& path, const char* target, ID3DBlob** shaderBlob)
{
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG |
		D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	const HRESULT result = D3DCompileFromFile(
		path.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		target,
		flags,
		0,
		shaderBlob,
		errorBlob.GetAddressOf());
	if (FAILED(result))
	{
		PrintShaderError(errorBlob.Get());
		return false;
	}

	return true;
}

bool PixelGraphics::ShaderFactory::CompileResource(const wchar_t* resourceName, const char* target, ID3DBlob** shaderBlob)
{
	const HMODULE module = GetPixelGraphicsModule();
	const HRSRC resource =
		FindResourceW(module, resourceName, L"HLSL");
	if (!resource)
	{
		return false;
	}

	const HGLOBAL resourceData = LoadResource(module, resource);
	const void* source = LockResource(resourceData);
	const DWORD sourceSize = SizeofResource(module, resource);
	if (!resourceData || !source || sourceSize == 0)
	{
		return false;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	const HRESULT result = D3DCompile(
		source,
		sourceSize,
		nullptr,
		nullptr,
		nullptr,
		"main",
		target,
		D3DCOMPILE_ENABLE_STRICTNESS,
		0,
		shaderBlob,
		errorBlob.GetAddressOf());
	if (FAILED(result))
	{
		PrintShaderError(errorBlob.Get());
		return false;
	}

	return true;
}

bool PixelGraphics::ShaderFactory::CreateShaderResources(ID3DBlob* vertexShaderBlob, ID3DBlob* pixelShaderBlob, ShaderResources* shader)
{
	ID3D11Device* device =
		graphicsCore ? graphicsCore->GetDevice() : nullptr;
	if (!device || !vertexShaderBlob ||
		!pixelShaderBlob || !shader)
	{
		return false;
	}

	HRESULT result = device->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(),
		nullptr,
		&shader->mVertexShader);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize(),
		nullptr,
		&shader->mPixelShader);
	if (FAILED(result))
	{
		return false;
	}

	if (!CreateConstantBuffers(vertexShaderBlob))
	{
		return false;
	}

	return CreateInputLayout(vertexShaderBlob, &shader->mLayout);
}

bool PixelGraphics::ShaderFactory::CreateConstantBuffers(ID3DBlob* shaderBlob)
{
	if (!graphicsCore || !graphicsCore->GetDevice() || !shaderBlob)
	{
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
	if (FAILED(D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, reinterpret_cast<void**>(reflection.GetAddressOf()))))
	{
		return false;
	}

	D3D11_SHADER_DESC shaderDescription = {};
	if (FAILED(reflection->GetDesc(&shaderDescription)))
	{
		return false;
	}

	for (UINT index = 0; index < shaderDescription.ConstantBuffers; ++index)
	{
		ID3D11ShaderReflectionConstantBuffer* reflectedBuffer = reflection->GetConstantBufferByIndex(index);
		if (!reflectedBuffer)
		{
			return false;
		}

		D3D11_SHADER_BUFFER_DESC bufferDescription = {};
		if (FAILED(reflectedBuffer->GetDesc(&bufferDescription)) || !bufferDescription.Name)
		{
			return false;
		}

		if (constantBuffers.find(bufferDescription.Name) != constantBuffers.end())
		{
			continue;
		}

		D3D11_BUFFER_DESC description = {};
		description.Usage = D3D11_USAGE_DEFAULT;
		description.ByteWidth = (bufferDescription.Size + 15u) & ~15u;
		description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		auto buffer = std::make_unique<BufferResources>();
		buffer->name = bufferDescription.Name;
		buffer->byteWidth = description.ByteWidth;
		if (FAILED(graphicsCore->GetDevice()->CreateBuffer(&description, nullptr, buffer->buffer.GetAddressOf())))
		{
			return false;
		}

		constantBuffers.emplace(buffer->name, std::move(buffer));
	}

	return true;
}

bool PixelGraphics::ShaderFactory::CreateInputLayout(ID3DBlob* vertexShaderBlob, ID3D11InputLayout** inputLayout)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
	HRESULT result = D3DReflect(
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		reinterpret_cast<void**>(
			reflection.GetAddressOf()));
	if (FAILED(result))
	{
		return false;
	}

	D3D11_SHADER_DESC shaderDesc = {};
	if (FAILED(reflection->GetDesc(&shaderDesc)))
	{
		return false;
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
	elements.reserve(shaderDesc.InputParameters);

	for (UINT index = 0;
		index < shaderDesc.InputParameters;
		++index)
	{
		D3D11_SIGNATURE_PARAMETER_DESC parameter = {};
		if (FAILED(reflection->GetInputParameterDesc(
			index,
			&parameter)))
		{
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC element = {};
		element.SemanticName = parameter.SemanticName;
		element.SemanticIndex = parameter.SemanticIndex;
		element.Format = GetInputFormat(
			parameter.ComponentType,
			parameter.Mask);
		element.InputSlot = 0;
		element.AlignedByteOffset =
			D3D11_APPEND_ALIGNED_ELEMENT;
		element.InputSlotClass =
			D3D11_INPUT_PER_VERTEX_DATA;
		element.InstanceDataStepRate = 0;
		elements.push_back(element);
	}

	result = graphicsCore->GetDevice()->CreateInputLayout(
		elements.data(),
		static_cast<UINT>(elements.size()),
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize(),
		inputLayout);
	return SUCCEEDED(result);
}

void PixelGraphics::ShaderFactory::ReleaseShader(ShaderResources* shader)
{
	if (!shader)
	{
		return;
	}

	if (shader->mLayout)
	{
		shader->mLayout->Release();
		shader->mLayout = nullptr;
	}
	if (shader->mPixelShader)
	{
		shader->mPixelShader->Release();
		shader->mPixelShader = nullptr;
	}
	if (shader->mVertexShader)
	{
		shader->mVertexShader->Release();
		shader->mVertexShader = nullptr;
	}
}

ShaderResources* PixelGraphics::ShaderFactory::Get(std::uint16_t key)
{
	const auto found = shaders.find(key);
	if (found != shaders.end())
	{
		return found->second.get();
	}

	const auto defaultShader = shaders.find(defaultShaderKey);
	if (defaultShader != shaders.end())
	{
		return defaultShader->second.get();
	}

	return nullptr;
}

BufferResources* PixelGraphics::ShaderFactory::GetBuffer(const std::string& name)
{
	const auto found = constantBuffers.find(name);
	return found != constantBuffers.end() ? found->second.get() : nullptr;
}
