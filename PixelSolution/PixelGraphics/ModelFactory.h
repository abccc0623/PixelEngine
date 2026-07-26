#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "InputLayout.h"
#include "ModelResources.h"
#include "ResourceFactory.h"
#include "GraphicsCore.h"

namespace PixelGraphics
{
	class GraphicsCore;

	class ModelFactory : public ResourceFactory
	{
	public:
		bool Initialize(GraphicsCore* graphicsCore) override;
		void Release() override;
		void Clear() override;
		std::uint16_t Load(const std::string& name) override;
		DirectModel* Get(std::uint16_t key);

	private:
		std::unique_ptr<DirectModel> CreateQuad();
		std::unique_ptr<DirectModel> Create2DBox();

		template<typename VertexType, typename IndexType>
		std::unique_ptr<DirectModel> CreateModelBuffer(VertexType* vertexArray, int vertexArraySize, IndexType* indexArray, int indexArraySize);

		template<typename T>
		Microsoft::WRL::ComPtr<ID3D11Buffer> CreateVertexBuffer(T* vertexArray, int vertexArraySize);

		template<typename T>
		Microsoft::WRL::ComPtr<ID3D11Buffer> CreateIndexBuffer(T* indexArray, int indexArraySize);

		GraphicsCore* graphicsCore = nullptr;
		std::uint16_t defaultModelKey = 0;
		std::unordered_map<std::uint16_t, DirectModel> models;
	};
}

template<typename VertexType, typename IndexType>
std::unique_ptr<DirectModel> PixelGraphics::ModelFactory::CreateModelBuffer(VertexType* vertexArray, int vertexArraySize, IndexType* indexArray, int indexArraySize)
{
	auto model = std::make_unique<DirectModel>();
	model->VertexBuffer = CreateVertexBuffer(vertexArray, vertexArraySize);
	model->IndexBuffer = CreateIndexBuffer(indexArray, indexArraySize);
	if (!model->VertexBuffer || !model->IndexBuffer)
	{
		return nullptr;
	}

	model->Offset = 0;
	model->stride = sizeof(VertexType);
	model->IndexCount = indexArraySize;
	model->VertexCount = vertexArraySize;
	return model;
}

template<typename T>
Microsoft::WRL::ComPtr<ID3D11Buffer> PixelGraphics::ModelFactory::CreateVertexBuffer(T* vertexArray, int vertexArraySize)
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	if (!graphicsCore || !graphicsCore->GetDevice())
	{
		return vertexBuffer;
	}

	D3D11_BUFFER_DESC description = {};
	description.Usage = D3D11_USAGE_IMMUTABLE;
	description.ByteWidth = static_cast<UINT>(sizeof(T) * vertexArraySize);
	description.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = vertexArray;
	graphicsCore->GetDevice()->CreateBuffer(&description, &initialData, vertexBuffer.GetAddressOf());
	return vertexBuffer;
}

template<typename T>
Microsoft::WRL::ComPtr<ID3D11Buffer> PixelGraphics::ModelFactory::CreateIndexBuffer(T* indexArray, int indexArraySize)
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	if (!graphicsCore || !graphicsCore->GetDevice())
	{
		return indexBuffer;
	}

	D3D11_BUFFER_DESC description = {};
	description.Usage = D3D11_USAGE_IMMUTABLE;
	description.ByteWidth = static_cast<UINT>(sizeof(T) * indexArraySize);
	description.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = indexArray;
	graphicsCore->GetDevice()->CreateBuffer(&description, &initialData, indexBuffer.GetAddressOf());
	return indexBuffer;
}
