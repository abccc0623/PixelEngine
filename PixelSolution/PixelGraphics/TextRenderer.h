#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "RenderringData.h"

namespace PixelGraphics
{
	class GraphicsCore;
	class ResourceManager;
	class CameraManager;
	class TextRenderer
	{
	public:
		TextRenderer(GraphicsCore* core, ResourceManager* resources, CameraManager* cameraManager);
		~TextRenderer();

		void Draw(const RenderingData& data);
	private:
		bool Initialize();
		bool BuildBuffers();
		void DrawText(const RenderingData& data);

		GraphicsCore* core = nullptr;
		ResourceManager* resources = nullptr;
		CameraManager* cameraManager = nullptr;
		bool initialized = false;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	};
}
