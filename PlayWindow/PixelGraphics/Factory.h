#pragma once
#include "GraphicsCore.h"
#include "ModelResources.h"

struct Vertex_Debug;
class Factory : GraphicsCore
{
public:
	Factory();
	virtual ~Factory();

	void Initialize();
	void Release();
	DirectModel* DebugAxis();
	DirectModel* DebugQuad();

	RenderingData* CreateRenderingData();
	void DeleteRenderingData(RenderingData* Data);
	int CreateDebugModel(float* VertexList, int VertexSize, int* IndexList, int IndexSize);
	int CreateDebugModel(Vertex_Debug* VertexList, int VertexSize, int* IndexList, int IndexSize);


	ID3D11RasterizerState* CreateRasterizerState_Solid();
	ID3D11RasterizerState* CreateRasterizerState_Wire();
public:
	template<typename VertextType, typename IndexType>
	DirectModel* CreateModelBuffer(VertextType* VertexArray, int VertexArraySize, IndexType* IndexArray, int IndexArraySize);
	template<typename T>
	ID3D11Buffer* CreateVertexBuffer(T* VertexArray, int VertexArraySize);
	template<typename T>
	ID3D11Buffer* CreateIndexBuffer(T* IndexArray, int IndexArraySize);
};

template<typename VertextType, typename IndexType>
inline DirectModel* Factory::CreateModelBuffer(VertextType* VertexArray, int VertexArraySize, IndexType* IndexArray, int IndexArraySize)
{
	DirectModel* mMode = new DirectModel();
	mMode->VertexBuffer = CreateVertexBuffer(VertexArray, VertexArraySize);
	mMode->IndexBuffer	= CreateIndexBuffer(IndexArray, IndexArraySize);

	mMode->Offset = 0;
	mMode->stride = sizeof(VertextType);
	mMode->IndexCount = IndexArraySize;
	mMode->VertexCount = VertexArraySize;
	return mMode;
}

template<typename T>
inline ID3D11Buffer* Factory::CreateVertexBuffer(T* VertexArray, int VertexArraySize)
{
	ID3D11Buffer* VertexBuffer = nullptr;
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(T) * VertexArraySize;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &VertexArray[0];
	GraphicsCore::GetDevice()->CreateBuffer(&vbd, &vinitData, &VertexBuffer);
	return VertexBuffer;
}

template<typename T>
inline ID3D11Buffer* Factory::CreateIndexBuffer(T* IndexArray, int IndexArraySize)
{
	ID3D11Buffer* IndexBuffer = nullptr;
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(T) * IndexArraySize;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &IndexArray[0];
	GraphicsCore::GetDevice()->CreateBuffer(&ibd, &iinitData, &IndexBuffer);
	return IndexBuffer;
}

