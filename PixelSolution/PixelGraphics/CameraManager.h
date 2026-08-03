#pragma once
#include "PixelResources.h"
#include "SimpleMath.h"
struct RenderingData;
namespace PixelGraphics
{
	class GraphicsCore;
	class ResourceManager;
	class ShaderFactory;
	class CameraManager
	{
	public:
		CameraManager(GraphicsCore* core, ResourceManager* resourceManager);
		~CameraManager();
	public:
		void ViewPortSetting(RenderingData& r);
		void Setting(RenderingData& r);
		DirectX::SimpleMath::Matrix GetView();
		DirectX::SimpleMath::Matrix GetProj();
		DirectX::SimpleMath::Matrix GetProjUI();
		void BindViewPort();
	private:
		GraphicsCore* core = nullptr;
		ResourceManager* resourceManager = nullptr;
		ShaderFactory* shaderFactory = nullptr;
		BufferResources* cameraBufferResource;
		D3D11_VIEWPORT targetViewport;

		DirectX::SimpleMath::Matrix view = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix proj = DirectX::SimpleMath::Matrix::Identity;
	};
}

