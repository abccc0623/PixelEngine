#include "pch.h"
#include "TextureFactory.h"
#include "GraphicsCore.h"
#include "ResourceManager.h"

bool PixelGraphics::TextureFactory::Initialize(GraphicsCore* graphicsCore)
{
	Clear();
	this->graphicsCore = graphicsCore;
	defaultTextureKey =
		static_cast<std::uint16_t>(ResourceDefaultKey::TEXTURE);
	return LoadDefaultTexture();
}

void PixelGraphics::TextureFactory::Release()
{
	Clear();
	graphicsCore = nullptr;
}

void PixelGraphics::TextureFactory::Clear()
{
	textures.clear();
	nextTextureKey = 1;
}

std::uint16_t PixelGraphics::TextureFactory::AllocateKey()
{
	constexpr std::uint16_t invalidKey =
		(std::numeric_limits<std::uint16_t>::max)();

	for (std::uint32_t count = 0; count < invalidKey - 1; ++count)
	{
		if (nextTextureKey == defaultTextureKey ||
			nextTextureKey == invalidKey)
		{
			nextTextureKey = 1;
		}

		const std::uint16_t key = nextTextureKey++;
		if (textures.find(key) == textures.end())
		{
			return key;
		}
	}

	return invalidKey;
}

std::uint16_t PixelGraphics::TextureFactory::Load(const std::string& path)
{
	for (const auto& [key, texture] : textures)
	{
		if (texture.path == path)
		{
			return key;
		}
	}

	ID3D11Device* device =
		graphicsCore ? graphicsCore->GetDevice() : nullptr;
	if (!device)
	{
		return defaultTextureKey;
	}

	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_uc* pixels = stbi_load(
		path.c_str(),
		&width,
		&height,
		&channels,
		STBI_rgb_alpha);
	if (!pixels)
	{
		return defaultTextureKey;
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = static_cast<UINT>(width);
	textureDesc.Height = static_cast<UINT>(height);
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = pixels;
	initialData.SysMemPitch = static_cast<UINT>(width * 4);

	ComPtr<ID3D11Texture2D> texture;
	const HRESULT textureResult = device->CreateTexture2D(
		&textureDesc,
		&initialData,
		texture.GetAddressOf());
	stbi_image_free(pixels);

	if (FAILED(textureResult))
	{
		return defaultTextureKey;
	}

	ComPtr<ID3D11ShaderResourceView> textureView;
	const HRESULT viewResult = device->CreateShaderResourceView(
		texture.Get(),
		nullptr,
		textureView.GetAddressOf());
	if (FAILED(viewResult))
	{
		return defaultTextureKey;
	}

	const std::uint16_t key = AllocateKey();
	constexpr std::uint16_t invalidKey =
		(std::numeric_limits<std::uint16_t>::max)();
	if (key == invalidKey)
	{
		return defaultTextureKey;
	}

	TextureResources textureResource = {};
	textureResource.key = key;
	textureResource.path = path;
	textureResource.Texture = std::move(textureView);
	textures.emplace(key, std::move(textureResource));
	return key;
}

bool PixelGraphics::TextureFactory::LoadDefaultTexture()
{
	ID3D11Device* device =
		graphicsCore ? graphicsCore->GetDevice() : nullptr;
	if (!device)
	{
		return false;
	}

	const HMODULE module = GetPixelGraphicsModule();
	const HRSRC resource =
		FindResourceW(module, L"DEFAULT_PNG", L"PNG");
	if (!resource)
	{
		return false;
	}

	const HGLOBAL resourceData = LoadResource(module, resource);
	const void* bytes = LockResource(resourceData);
	const DWORD byteCount = SizeofResource(module, resource);
	if (!resourceData || !bytes || byteCount == 0)
	{
		return false;
	}

	int width = 0;
	int height = 0;
	int channelCount = 0;
	stbi_uc* pixels = stbi_load_from_memory(
		static_cast<const stbi_uc*>(bytes),
		static_cast<int>(byteCount),
		&width,
		&height,
		&channelCount,
		STBI_rgb_alpha);
	if (!pixels)
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = static_cast<UINT>(width);
	textureDesc.Height = static_cast<UINT>(height);
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = pixels;
	initialData.SysMemPitch = static_cast<UINT>(width * 4);

	ComPtr<ID3D11Texture2D> texture;
	const HRESULT textureResult = device->CreateTexture2D(
		&textureDesc,
		&initialData,
		texture.GetAddressOf());
	stbi_image_free(pixels);

	if (FAILED(textureResult))
	{
		return false;
	}

	ComPtr<ID3D11ShaderResourceView> textureView;
	const HRESULT viewResult = device->CreateShaderResourceView(
		texture.Get(),
		nullptr,
		textureView.GetAddressOf());
	if (FAILED(viewResult))
	{
		return false;
	}

	TextureResources textureResource = {};
	textureResource.key = defaultTextureKey;
	textureResource.path = "DEFAULT_PNG";
	textureResource.Texture = std::move(textureView);
	textures.emplace(defaultTextureKey, std::move(textureResource));
	return true;
}

TextureResources* PixelGraphics::TextureFactory::Get(std::uint16_t key)
{
	const auto found = textures.find(key);
	if (found != textures.end())
	{
		return &found->second;
	}

	const auto defaultTexture = textures.find(defaultTextureKey);
	if (defaultTexture != textures.end())
	{
		return &defaultTexture->second;
	}

	return nullptr;
}
