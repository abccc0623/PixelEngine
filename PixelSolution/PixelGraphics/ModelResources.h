#pragma once
struct ID3D11Buffer;
using Handle16 = unsigned short;
using Handle32 = unsigned int;
using Handle64 = unsigned long long;
struct DirectModel
{
	Handle16 key;
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	unsigned int stride = 0;
	unsigned int Offset = 0;

	int VertexCount = 0;
	int IndexCount  = 0;
};