#pragma once
#include <cstdint>
#include "InputLayout.h"
#include <string>
#include <unordered_map>
#include <d3d11.h>
#include <wrl/client.h>
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11InputLayout;
struct ID3D11RasterizerState;
struct ID3D11ShaderResourceView;
struct ID3D11Buffer;
using Handle32 = unsigned int;
using Handle64 = unsigned long long;

struct ShaderResources
{
	std::uint16_t key;
	std::string path;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11InputLayout* mLayout = nullptr;
};

struct RasterizerStateResources
{
	std::uint16_t key;
	std::string path;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
};

struct TextureResources
{
	std::uint16_t key;
	std::string path;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Texture;
};

struct BufferResources
{
	std::uint16_t key;
	std::string name;
	UINT byteWidth = 0;
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
};

struct MaterialResources
{
	std::uint16_t key;
	std::string path;
	std::uint16_t TextureKey;
	std::uint16_t ShaderKey;
	std::uint16_t RasterizerStateKey;
	float Offset[2] = { 1,1 };
	float Tiling[2] = { 1,1 };
};

struct GlyphData
{
	float u0 = 0.0f;
	float v0 = 0.0f;
	float u1 = 0.0f;
	float v1 = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	float offsetX = 0.0f;
	float offsetY = 0.0f;
	float advance = 0.0f;
};

struct FontResources
{
	std::uint16_t key = 0;
	std::string path;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> atlasTexture;
	float fontSize = 0.0f;
	float ascent = 0.0f;
	float descent = 0.0f;
	float lineGap = 0.0f;
	float lineHeight = 0.0f;
	std::unordered_map<char32_t, GlyphData> glyphs;
};

struct RenderTarget
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	int width = 0;
	int height = 0;
};
