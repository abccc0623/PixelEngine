#pragma once
struct ID3D11Buffer;
struct DirectModel
{
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	unsigned int stride = 0;
	unsigned int Offset = 0;

	int VertexCount = 0;
	int IndexCount  = 0;
};