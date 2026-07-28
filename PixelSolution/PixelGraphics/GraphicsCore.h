#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include "SimpleMath.h"
#include "PixelResources.h"

namespace PixelGraphics
{
	class GraphicsCore
	{
	public:
		GraphicsCore() = default;
		~GraphicsCore();

		GraphicsCore(const GraphicsCore&) = delete;
		GraphicsCore& operator=(const GraphicsCore&) = delete;

		bool Initialize(HWND windowHandle);
		bool Resize(int width, int height);
		void Release();

		void BindBackBuffer();
		void ClearBackBuffer(float red, float green, float blue, float alpha);
		void ApplyViewport();
		void Present();
		bool CreateRenderTarget(int width, int height, RenderTarget& renderTarget);

		ID3D11Device* GetDevice() const;
		ID3D11DeviceContext* GetDeviceContext() const;
		IDXGISwapChain* GetSwapChain() const;
		ID3D11RenderTargetView* GetBackBufferRTV() const;
		ID3D11DepthStencilView* GetDepthStencilView() const;
		ID3D11DepthStencilState* GetDepthEnabledState() const;
		ID3D11DepthStencilState* GetDepthDisabledState() const;
		ID3D11BlendState* GetAlphaBlendState() const;

		int GetClientWidth() const;
		int GetClientHeight() const;

		const DirectX::SimpleMath::Matrix& GetView() const;
		const DirectX::SimpleMath::Matrix& GetProjection() const;
		void SetView(const DirectX::SimpleMath::Matrix& view);
		void SetProjection(const DirectX::SimpleMath::Matrix& projection);
	private:
		bool CreateDeviceAndSwapChain();
		bool CreateBackBuffer();
		bool CreateDepthStencil();
		bool CreateDepthStencilStates();
		bool CreateBlendState();
		void UpdateViewport();
		void ReleaseSizeDependentResources();
	private:
		HWND windowHandle = nullptr;
		int clientWidth = 0;
		int clientHeight = 0;

		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_UNKNOWN;
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		D3D11_VIEWPORT screenViewport = {};

		Microsoft::WRL::ComPtr<ID3D11Device> device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthEnabledState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthDisabledState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> alphaBlendState;

		DirectX::SimpleMath::Matrix view = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix projection = DirectX::SimpleMath::Matrix::Identity;
	};
}
