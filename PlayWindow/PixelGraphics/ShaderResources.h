#pragma once

struct ID3D11PixelShader;
struct ID3D11VertexShader;
struct ID3D11InputLayout;

struct ShaderResources
{
	ID3D11PixelShader*	mPixelShader	= nullptr;
	ID3D11VertexShader* mVertexShader	= nullptr;
	ID3D11InputLayout*  mLayout			= nullptr;
};