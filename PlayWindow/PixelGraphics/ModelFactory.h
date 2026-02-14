#pragma once
#include "PixelResources.h"
#include "ResourceFactory.h"
#include "GraphicsCore.h"
#include "ModelResources.h"
#include <d3d11.h>
struct StaticModel;
struct DirectModel;
class ModelFactory : public ResourceFactory
{
public:
	ModelFactory() = default;
	virtual ~ModelFactory() = default;

	virtual void Initialize();
	virtual void Release();
protected:
	virtual void* GetResource(std::string name);

	DirectModel* CreateQuad();
	std::unordered_map<Handle16, DirectModel*> ModelMap;
	std::unordered_map<std::string, DirectModel*> defaultModelMap;


	template<typename VertextType, typename IndexType>
	DirectModel* CreateModelBuffer(VertextType* VertexArray, int VertexArraySize, IndexType* IndexArray, int IndexArraySize);
	template<typename T>
	ID3D11Buffer* CreateVertexBuffer(T* VertexArray, int VertexArraySize);
	template<typename T>
	ID3D11Buffer* CreateIndexBuffer(T* IndexArray, int IndexArraySize);
};

template<typename VertextType, typename IndexType>
inline DirectModel* ModelFactory::CreateModelBuffer(VertextType* VertexArray, int VertexArraySize, IndexType* IndexArray, int IndexArraySize)
{
	DirectModel* mMode = new DirectModel();
	mMode->VertexBuffer = CreateVertexBuffer(VertexArray, VertexArraySize);
	mMode->IndexBuffer = CreateIndexBuffer(IndexArray, IndexArraySize);
	
	mMode->Offset = 0;
	mMode->stride = sizeof(VertextType);
	mMode->IndexCount = IndexArraySize;
	mMode->VertexCount = VertexArraySize;
	return mMode;
}

template<typename T>
inline ID3D11Buffer* ModelFactory::CreateVertexBuffer(T* VertexArray, int VertexArraySize)
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
inline ID3D11Buffer* ModelFactory::CreateIndexBuffer(T* IndexArray, int IndexArraySize)
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