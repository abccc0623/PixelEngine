#include <Windows.h>
#include "TextureFactory.h"
#include <d3d11.h>
#include "GraphicsCore.h"
#include <unordered_map>
#include "PixelGraphicsLib.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
TextureFactory::TextureFactory()
{
	textureMap = std::unordered_map<Handle16, TextureResources*>();
}

TextureFactory::~TextureFactory()
{
}

void TextureFactory::Initialize()
{
	defaulttextureMap.insert({"Default",DefaultTextureResource(L"DEFAULT_PNG")});
	keyAllocator.GetKey16();
}

void TextureFactory::Release()
{
	for (auto& k : textureMap)
	{
		k.second->Texture->Release();
		delete k.second;
		k.second = nullptr;
	}
	textureMap.clear();

	for (auto& k : defaulttextureMap)
	{
		k.second->Texture->Release();
		delete k.second;
		k.second = nullptr;
	}
	defaulttextureMap.clear();
}


TextureResources* TextureFactory::CreateTextureResource(const char* filePath)
{
	int width, height, channels;
	// 1. 이미지를 CPU 메모리로 로드 (RGBA 강제)
	unsigned char* pixels = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
		std::cout << "Not Find Texture" << std::endl;
		return nullptr;
	}

	// 2. Texture2D 설명 설정
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 일반적인 32비트 색상 포맷
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// 3. 초기 데이터 설정 (CPU 픽셀 -> GPU 초기화 데이터)
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pixels;
	initData.SysMemPitch = width * 4; // RGBA니까 너비 * 4바이트

	ID3D11Texture2D* pTexture = nullptr;

	GraphicsCore::GetDevice()->CreateTexture2D(&texDesc, &initData, &pTexture);
	// 4. Shader Resource View(SRV) 생성
	ID3D11ShaderResourceView* pSRV = nullptr;
	if (pTexture) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		GraphicsCore::GetDevice()->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
		pTexture->Release(); // SRV가 내부 참조를 가지므로 Texture 객체 자체는 해제 가능
	}

	stbi_image_free(pixels); // CPU 메모리 해제
	auto  texture = new TextureResources();
	texture->Texture = pSRV;
	texture->key = keyAllocator.GetKey16();
	return texture;
}


TextureResources* TextureFactory::DefaultTextureResource(const wchar_t* resourceName)
{
	HMODULE hModule = GetModuleHandle(L"PixelGraphics.dll");
	HRSRC hRes = FindResource(hModule, resourceName, L"PNG");
	if (!hRes) return nullptr;

	HGLOBAL hData = LoadResource(hModule, hRes);
	void* pData = LockResource(hData);
	DWORD size = SizeofResource(hModule, hRes);

	if (!hData) return nullptr;

	int width, height, channels;
	unsigned char* imageLogit = stbi_load_from_memory((stbi_uc*)pData, size, &width, &height, &channels, 4);

	if (!imageLogit)
	{
		std::cout << "Not Find Texture" << std::endl;
		return nullptr;
	}

	// 2. Texture2D 설명 설정
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 일반적인 32비트 색상 포맷
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// 3. 초기 데이터 설정 (CPU 픽셀 -> GPU 초기화 데이터)
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = imageLogit;
	initData.SysMemPitch = width * 4; // RGBA니까 너비 * 4바이트

	ID3D11Texture2D* pTexture = nullptr;

	GraphicsCore::GetDevice()->CreateTexture2D(&texDesc, &initData, &pTexture);
	// 4. Shader Resource View(SRV) 생성
	ID3D11ShaderResourceView* pSRV = nullptr;
	if (pTexture) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		GraphicsCore::GetDevice()->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
		pTexture->Release(); // SRV가 내부 참조를 가지므로 Texture 객체 자체는 해제 가능
	}

	stbi_image_free(imageLogit); // CPU 메모리 해제

	auto  texture = new TextureResources();
	texture->Texture = pSRV;
	return texture;
}


void* TextureFactory::GetResource(std::string name)
{
	auto value = defaulttextureMap.find(name);
	if (value != defaulttextureMap.end())
	{
		return value->second;
	}
	return nullptr;
}

void* TextureFactory::GetResource(Handle16 key)
{
	auto value = textureMap.find(key);
	if (value != textureMap.end())
	{
		return value->second;
	}
	return nullptr;
}

Handle16 TextureFactory::SetResource(std::string name)
{
	auto value = CreateTextureResource(name.c_str());
	textureMap.insert({ value->key, value });
	return value->key;
}
