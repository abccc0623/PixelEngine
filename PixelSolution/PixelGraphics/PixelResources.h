#pragma once
#include "InputLayout.h"
struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11InputLayout;
struct ID3D11RasterizerState;
struct ID3D11ShaderResourceView;
struct ID3D11Buffer;

using Handle16 = unsigned short;
using Handle32 = unsigned int;
using Handle64 = unsigned long long;

struct ShaderResources
{
	Handle16 key;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11InputLayout* mLayout = nullptr;
};

struct RasterizerStateResources
{
	Handle16 key;
	ID3D11RasterizerState* rasterizerState = nullptr;
};

struct TextureResources
{
	Handle16 key;
	ID3D11ShaderResourceView* Texture = nullptr;
};

struct BufferResources
{
	Handle16 key;
	ID3D11Buffer* buffer = nullptr;
};
