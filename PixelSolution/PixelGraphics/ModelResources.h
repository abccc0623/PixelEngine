#pragma once
#include <cstdint>
#include <string>
#include <d3d11.h>
#include <wrl/client.h>
using Handle32 = unsigned int;
using Handle64 = unsigned long long;
struct DirectModel
{
	std::uint16_t key;
	std::string path;
	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;

	unsigned int stride = 0;
	unsigned int Offset = 0;

	int VertexCount = 0;
	int IndexCount  = 0;
};
