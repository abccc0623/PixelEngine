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

	class Pipeline
	{
	public:
		Pipeline(GraphicsCore* core, ResourceManager* resourceManager);
		virtual ~Pipeline() = default;
		virtual bool Initialize() = 0;
		virtual void Render(std::vector<RenderingData>& renderingList, const float* backgroundColor) = 0;
		virtual void Release();
		virtual void SetCamera(RenderingData& camera);


		DirectX::SimpleMath::Matrix& GetView();
		DirectX::SimpleMath::Matrix& GetProjection();
	protected:
		GraphicsCore* core = nullptr;
		ResourceManager* resourceManager = nullptr;
		TextureFactory* textureFactory = nullptr;
		ShaderFactory* shaderFactory = nullptr;
		RasterizerStateFactory* rasterizerStateFactory = nullptr;
		ModelFactory* modelFactory = nullptr;
		MaterialFactory* materialFactory = nullptr;

		Microsoft::WRL::ComPtr<ID3D11Buffer> cameraBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> objectBuffer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

		DirectX::SimpleMath::Matrix view;
		DirectX::SimpleMath::Matrix projection;
	};
}

