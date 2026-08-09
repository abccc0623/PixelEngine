#include "pch.h"
#include "GraphicsCore.h"

using Microsoft::WRL::ComPtr;

PixelGraphics::GraphicsCore::~GraphicsCore()
{
	Release();
}

bool PixelGraphics::GraphicsCore::Initialize(HWND targetWindowHandle)
{
	if (targetWindowHandle == nullptr)
	{
		return false;
	}

	RECT clientRect = {};
	if (!GetClientRect(targetWindowHandle, &clientRect))
	{
		return false;
	}

	const int width = clientRect.right - clientRect.left;
	const int height = clientRect.bottom - clientRect.top;
	if (width <= 0 || height <= 0)
	{
		return false;
	}

	Release();

	windowHandle = targetWindowHandle;
	clientWidth = width;
	clientHeight = height;

	if (!CreateDeviceAndSwapChain() || !CreateBackBuffer() || !CreateDepthStencil() || !CreateDepthStencilStates() || !CreateBlendState())
	{
		Release();
		return false;
	}

	UpdateViewport();
	return true;
}

bool PixelGraphics::GraphicsCore::CreateDeviceAndSwapChain()
{
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	const D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};

	const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = clientWidth;
	swapChainDesc.BufferDesc.Height = clientHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = windowHandle;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	for (const D3D_DRIVER_TYPE candidateDriverType : driverTypes)
	{
		driverType = candidateDriverType;

		const HRESULT result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			driverType,
			nullptr,
			createDeviceFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&swapChainDesc,
			swapChain.ReleaseAndGetAddressOf(),
			device.ReleaseAndGetAddressOf(),
			&featureLevel,
			deviceContext.ReleaseAndGetAddressOf());

		if (SUCCEEDED(result))
		{
			return true;
		}
	}

	driverType = D3D_DRIVER_TYPE_UNKNOWN;
	return false;
}

bool PixelGraphics::GraphicsCore::CreateBackBuffer()
{
	if (!device || !swapChain)
	{
		return false;
	}

	ComPtr<ID3D11Texture2D> backBuffer;
	HRESULT result = swapChain->GetBuffer(
		0,
		IID_PPV_ARGS(backBuffer.GetAddressOf()));

	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateRenderTargetView(
		backBuffer.Get(),
		nullptr,
		backBufferRTV.ReleaseAndGetAddressOf());

	return SUCCEEDED(result);
}

bool PixelGraphics::GraphicsCore::CreateRenderTarget(int width, int height, RenderTarget& renderTarget)
{
	if (!device || width <= 0 || height <= 0)
	{
		return false;
	}

	renderTarget = {};

	D3D11_TEXTURE2D_DESC description = {};
	description.Width = static_cast<UINT>(width);
	description.Height = static_cast<UINT>(height);
	description.MipLevels = 1;
	description.ArraySize = 1;
	description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.SampleDesc.Count = 1;
	description.Usage = D3D11_USAGE_DEFAULT;
	description.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(device->CreateTexture2D(&description, nullptr, renderTarget.texture.GetAddressOf())))
	{
		return false;
	}
	if (FAILED(device->CreateRenderTargetView(renderTarget.texture.Get(), nullptr, renderTarget.renderTargetView.GetAddressOf())))
	{
		renderTarget = {};
		return false;
	}
	if (FAILED(device->CreateShaderResourceView(renderTarget.texture.Get(), nullptr, renderTarget.shaderResourceView.GetAddressOf())))
	{
		renderTarget = {};
		return false;
	}

	D3D11_TEXTURE2D_DESC depthDescription = {};
	depthDescription.Width = static_cast<UINT>(width);
	depthDescription.Height = static_cast<UINT>(height);
	depthDescription.MipLevels = 1;
	depthDescription.ArraySize = 1;
	depthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDescription.SampleDesc.Count = 1;
	depthDescription.Usage = D3D11_USAGE_DEFAULT;
	depthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(device->CreateTexture2D(&depthDescription, nullptr, renderTarget.depthStencilTexture.GetAddressOf())))
	{
		renderTarget = {};
		return false;
	}
	if (FAILED(device->CreateDepthStencilView(renderTarget.depthStencilTexture.Get(), nullptr, renderTarget.depthStencilView.GetAddressOf())))
	{
		renderTarget = {};
		return false;
	}

	renderTarget.width = width;
	renderTarget.height = height;
	return true;
}

bool PixelGraphics::GraphicsCore::CreateDepthStencil()
{
	if (!device)
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = clientWidth;
	depthDesc.Height = clientHeight;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	HRESULT result = device->CreateTexture2D(
		&depthDesc,
		nullptr,
		depthStencilTexture.ReleaseAndGetAddressOf());

	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateDepthStencilView(
		depthStencilTexture.Get(),
		nullptr,
		depthStencilView.ReleaseAndGetAddressOf());

	return SUCCEEDED(result);
}

bool PixelGraphics::GraphicsCore::CreateDepthStencilStates()
{
	if (!device)
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC enabledDescription = {};
	enabledDescription.DepthEnable = TRUE;
	enabledDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	enabledDescription.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	enabledDescription.StencilEnable = FALSE;
	if (FAILED(device->CreateDepthStencilState(&enabledDescription, depthEnabledState.ReleaseAndGetAddressOf())))
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC disabledDescription = {};
	disabledDescription.DepthEnable = FALSE;
	disabledDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	disabledDescription.DepthFunc = D3D11_COMPARISON_ALWAYS;
	disabledDescription.StencilEnable = FALSE;
	if (FAILED(device->CreateDepthStencilState(&disabledDescription, depthDisabledState.ReleaseAndGetAddressOf())))
	{
		depthEnabledState.Reset();
		return false;
	}

	return true;
}

bool PixelGraphics::GraphicsCore::CreateBlendState()
{
	if (!device)
	{
		return false;
	}

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	D3D11_RENDER_TARGET_BLEND_DESC& targetBlend =
		blendDesc.RenderTarget[0];
	targetBlend.BlendEnable = TRUE;
	targetBlend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	targetBlend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	targetBlend.BlendOp = D3D11_BLEND_OP_ADD;
	targetBlend.SrcBlendAlpha = D3D11_BLEND_ONE;
	targetBlend.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	targetBlend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	targetBlend.RenderTargetWriteMask =
		D3D11_COLOR_WRITE_ENABLE_ALL;

	const HRESULT result = device->CreateBlendState(
		&blendDesc,
		alphaBlendState.ReleaseAndGetAddressOf());

	return SUCCEEDED(result);
}

void PixelGraphics::GraphicsCore::UpdateViewport()
{
	screenViewport.TopLeftX = 0.0f;
	screenViewport.TopLeftY = 0.0f;
	screenViewport.Width = static_cast<float>(clientWidth);
	screenViewport.Height = static_cast<float>(clientHeight);
	screenViewport.MinDepth = 0.0f;
	screenViewport.MaxDepth = 1.0f;

	if (deviceContext)
	{
		deviceContext->RSSetViewports(1, &screenViewport);
	}
}

void PixelGraphics::GraphicsCore::ReleaseSizeDependentResources()
{
	if (deviceContext)
	{
		deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	}

	depthStencilView.Reset();
	depthStencilTexture.Reset();
	backBufferRTV.Reset();
}

bool PixelGraphics::GraphicsCore::Resize(int width, int height)
{
	if (!device || !deviceContext || !swapChain ||
		width <= 0 || height <= 0)
	{
		return false;
	}

	if (width == clientWidth && height == clientHeight)
	{
		return true;
	}

	ReleaseSizeDependentResources();

	const HRESULT result = swapChain->ResizeBuffers(
		0,
		width,
		height,
		DXGI_FORMAT_UNKNOWN,
		0);

	if (FAILED(result))
	{
		return false;
	}

	clientWidth = width;
	clientHeight = height;

	if (!CreateBackBuffer() || !CreateDepthStencil())
	{
		return false;
	}

	UpdateViewport();

	ID3D11RenderTargetView* target = backBufferRTV.Get();
	deviceContext->OMSetRenderTargets(
		1,
		&target,
		depthStencilView.Get());

	return true;
}

void PixelGraphics::GraphicsCore::BindBackBuffer()
{
	if (!deviceContext || !backBufferRTV || !depthStencilView)
	{
		return;
	}

	ID3D11RenderTargetView* target = backBufferRTV.Get();
	deviceContext->OMSetRenderTargets(1, &target, depthStencilView.Get());
}

void PixelGraphics::GraphicsCore::ClearBackBuffer(float red, float green, float blue, float alpha)
{
	if (!deviceContext || !backBufferRTV || !depthStencilView)
	{
		return;
	}

	const float clearColor[4] = { red, green, blue, alpha };
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContext->ClearRenderTargetView(backBufferRTV.Get(), clearColor);
}

void PixelGraphics::GraphicsCore::ApplyViewport()
{
	if (!deviceContext)
	{
		return;
	}

	deviceContext->RSSetViewports(1, &screenViewport);
}

void PixelGraphics::GraphicsCore::ApplyViewport(int contentWidth, int contentHeight)
{
	if (!deviceContext || clientWidth <= 0 || clientHeight <= 0 || contentWidth <= 0 || contentHeight <= 0)
	{
		return;
	}

	const float clientAspect = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);
	const float contentAspect = static_cast<float>(contentWidth) / static_cast<float>(contentHeight);
	D3D11_VIEWPORT viewport = {};
	if (clientAspect > contentAspect)
	{
		viewport.Height = static_cast<float>(clientHeight);
		viewport.Width = viewport.Height * contentAspect;
		viewport.TopLeftX = (static_cast<float>(clientWidth) - viewport.Width) * 0.5f;
	}
	else
	{
		viewport.Width = static_cast<float>(clientWidth);
		viewport.Height = viewport.Width / contentAspect;
		viewport.TopLeftY = (static_cast<float>(clientHeight) - viewport.Height) * 0.5f;
	}
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &viewport);
}

void PixelGraphics::GraphicsCore::Present()
{
	if (swapChain)
	{
		swapChain->Present(0, 0);
	}
}

void PixelGraphics::GraphicsCore::Release()
{
	if (deviceContext)
	{
		deviceContext->ClearState();
		deviceContext->Flush();
	}

	alphaBlendState.Reset();
	depthDisabledState.Reset();
	depthEnabledState.Reset();
	ReleaseSizeDependentResources();
	swapChain.Reset();
	deviceContext.Reset();
	device.Reset();

	windowHandle = nullptr;
	clientWidth = 0;
	clientHeight = 0;
	driverType = D3D_DRIVER_TYPE_UNKNOWN;
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	screenViewport = {};
	view = DirectX::SimpleMath::Matrix::Identity;
	projection = DirectX::SimpleMath::Matrix::Identity;
}

ID3D11Device* PixelGraphics::GraphicsCore::GetDevice() const
{
	return device.Get();
}

ID3D11DeviceContext* PixelGraphics::GraphicsCore::GetDeviceContext() const
{
	return deviceContext.Get();
}

IDXGISwapChain* PixelGraphics::GraphicsCore::GetSwapChain() const
{
	return swapChain.Get();
}

ID3D11RenderTargetView* PixelGraphics::GraphicsCore::GetBackBufferRTV() const
{
	return backBufferRTV.Get();
}

ID3D11DepthStencilView* PixelGraphics::GraphicsCore::GetDepthStencilView() const
{
	return depthStencilView.Get();
}

ID3D11DepthStencilState* PixelGraphics::GraphicsCore::GetDepthEnabledState() const
{
	return depthEnabledState.Get();
}

ID3D11DepthStencilState* PixelGraphics::GraphicsCore::GetDepthDisabledState() const
{
	return depthDisabledState.Get();
}

ID3D11BlendState* PixelGraphics::GraphicsCore::GetAlphaBlendState() const
{
	return alphaBlendState.Get();
}

int PixelGraphics::GraphicsCore::GetClientWidth() const
{
	return clientWidth;
}

int PixelGraphics::GraphicsCore::GetClientHeight() const
{
	return clientHeight;
}

const DirectX::SimpleMath::Matrix& PixelGraphics::GraphicsCore::GetView() const
{
	return view;
}

const DirectX::SimpleMath::Matrix& PixelGraphics::GraphicsCore::GetProjection() const
{
	return projection;
}

void PixelGraphics::GraphicsCore::SetView(const DirectX::SimpleMath::Matrix& newView)
{
	view = newView;
}

void PixelGraphics::GraphicsCore::SetProjection(const DirectX::SimpleMath::Matrix& newProjection)
{
	projection = newProjection;
}
