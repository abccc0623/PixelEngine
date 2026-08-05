#pragma once
#include "PixelResources.h"
#include "RenderringData.h"
namespace PixelGraphics
{
	class GraphicsCore;
	class ResourceManager;
	class CameraManager;
	class ShaderFactory;
	class TextureFactory;
	class ModelFactory;
	class MaterialFactory;
	class RasterizerStateFactory;
	class PASS
	{
	public:
		PASS(GraphicsCore* core, ResourceManager* resourceManager, CameraManager* cameraManager);
		virtual ~PASS();
		virtual void SetRenderingData(RenderingData& renderingData) = 0;
		virtual void Rendering(RenderTarget& renderTarget) = 0;

		virtual void BindObjectBuffer(RenderingData& r);
		virtual void BindTexture(RenderingData& r);
		virtual void BindShader(RenderingData& r);
		virtual int BindMesh(RenderingData& r);
		virtual void BindRasterizer(RenderingData& r);
		virtual void Sort(std::vector<RenderingData>& data);
	public:
		void CreateSampler();
		void PreviousKeyReset();
	protected:
		GraphicsCore* core = nullptr;
		ResourceManager* resourceManager = nullptr;
		CameraManager* cameraManager = nullptr;
		//Factory
		ShaderFactory* shaderFactory = nullptr;
		TextureFactory* textureFactory = nullptr;
		BufferResources* contextObjectBuffer = nullptr;
		ModelFactory* modelFactory = nullptr;
		MaterialFactory* materialFactory = nullptr;
		RasterizerStateFactory* rasterizerStateFactory = nullptr;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
		//key
		std::uint16_t previousMeshKey = UINT16_MAX;
		std::uint16_t previousMaterialKey = UINT16_MAX;
		std::uint16_t previousShaderKey = UINT16_MAX;
		std::uint16_t previousTextureKey = UINT16_MAX;
		std::uint16_t previousRasterizerKey = UINT16_MAX;
		int meshIndexCount = INT64_MAX;
	private:
	};
}

