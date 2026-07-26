#pragma once
#include <cstdint>
#include "InputLayout.h"
#include <string>
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
	ID3D11Buffer* buffer = nullptr;
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
