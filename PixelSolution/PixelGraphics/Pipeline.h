#pragma once
#include <vector>
#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>
#include "SimpleMath.h"

struct RenderingData;
namespace PixelGraphics
{
	class GraphicsCore;
	class ResourceManager;
	class TextureFactory;
	class ShaderFactory;
	class RasterizerStateFactory;
	class ModelFactory;
	class MaterialFactory;

	using namespace DirectX::SimpleMath;
	class Pipeline
	{
	public:
		Pipeline(GraphicsCore* core, ResourceManager* resourceManager);
		virtual ~Pipeline() = default;
		bool Initialize();
		void Release();
		virtual void PipelineInitialize() = 0;
		virtual void PipelineRelease() = 0;
		virtual void Rendering() = 0;
		virtual void SetRenderingData(RenderingData& renderingData) = 0;

		void SetBackgroundColor(float r, float g, float b);
		Matrix& GetCameraView();
		Matrix& GetCameraProjection();
		Matrix GetUIProjection() const;
	protected:
		virtual void Sort(std::vector<RenderingData>& data);
		virtual void SortUI(std::vector<RenderingData>& data);
		virtual void CameraSetting(RenderingData cameraData);
		void ApplyViewport();

		Microsoft::WRL::ComPtr<ID3D11Buffer> cameraBuffer;
		D3D11_VIEWPORT viewport = {};

	protected:
		GraphicsCore* core = nullptr;
		ResourceManager* resourceManager = nullptr;
		//리소스 팩토리들
		TextureFactory* textureFactory = nullptr;
		ShaderFactory* shaderFactory = nullptr;
		RasterizerStateFactory* rasterizerStateFactory = nullptr;
		ModelFactory* modelFactory = nullptr;
		MaterialFactory* materialFactory = nullptr;
	protected:
		bool IsCameraSetting = false;
		float backgroundColor[4] = { 1.0f,1.0f, 1.0f, 1.0f };
	private:
		Matrix view = DirectX::SimpleMath::Matrix::Identity;
		Matrix proj = DirectX::SimpleMath::Matrix::Identity;
	};
}

