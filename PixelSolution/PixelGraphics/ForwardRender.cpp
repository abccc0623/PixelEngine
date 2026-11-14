#include "pch.h"
#include "ForwardRender.h"
#include <cstring>
#include <wincodec.h>
#include "GraphicsCore.h"
#include "RenderringData.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "ScenePass.h"
#include "DebugLinePass.h"
#include "UIPass.h"
#include "PostProcessPass.h"

using namespace DirectX::SimpleMath;
PixelGraphics::ForwardRender::ForwardRender(GraphicsCore* graphicsCore, ResourceManager* resources) : Pipeline(graphicsCore, resources)
{
	cameraManager = new CameraManager(graphicsCore, resources);
	passList.push_back(new ScenePass(graphicsCore, resources, cameraManager));
	passList.push_back(new DebugLinePass(graphicsCore, resources, cameraManager));
	passList.push_back(new UIPass(graphicsCore, resources, cameraManager));
	postProcessPass = new PostProcessPass(graphicsCore, resources, cameraManager);
}
PixelGraphics::ForwardRender::~ForwardRender()
{
	PipelineRelease();
}

bool PixelGraphics::ForwardRender::PipelineInitialize()
{
	if (!cameraManager || !cameraManager->IsReady() || !postProcessPass || !postProcessPass->IsReady())
	{
		return false;
	}
	for (const auto* pass : passList)
	{
		if (!pass || !pass->IsReady()) return false;
	}
	return core->CreateRenderTarget(GraphicsCore::VirtualWidth, GraphicsCore::VirtualHeight, mainRenderTarget);
}

bool PixelGraphics::ForwardRender::Resize(int width, int height)
{
	if (width <= 0 || height <= 0)
	{
		return false;
	}

	return true;
}

void PixelGraphics::ForwardRender::PipelineRelease()
{
	mainRenderTarget = {};
	for (auto& K : passList)
	{
		delete K;
		K = nullptr;
	}
	delete postProcessPass;
	postProcessPass = nullptr;
	delete cameraManager;
	cameraManager = nullptr;
	passList.clear();
}

void PixelGraphics::ForwardRender::SetRenderingData(RenderingData& renderingData)
{
	if (renderingData.renderType == RENDER_TYPE::CAMERA)
	{
		cameraManager->Setting(renderingData);
	}
	else if (renderingData.renderType == RENDER_TYPE::LINE)
	{
		passList[1]->SetRenderingData(renderingData);
	}
	else if (renderingData.passType == PASS_TYPE::SCENE)
	{
		passList[0]->SetRenderingData(renderingData);

	}
	else if (renderingData.passType == PASS_TYPE::UI)
	{
		passList[2]->SetRenderingData(renderingData);
	}
	else
	{

	}
}


void PixelGraphics::ForwardRender::Rendering()
{
	ID3D11DeviceContext* context = core->GetDeviceContext();
	ID3D11RenderTargetView* sceneRTV = mainRenderTarget.renderTargetView.Get();
	ID3D11DepthStencilView* sceneDSV = mainRenderTarget.depthStencilView.Get();

	context->ClearDepthStencilView(sceneDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &sceneRTV, sceneDSV);
	cameraManager->BindViewPort();

	context->ClearRenderTargetView(sceneRTV, backgroundColor);
	for (auto& pass : passList)
	{
		pass->Rendering(mainRenderTarget);
	}

	//렌더 타깃 출력 연결 해제
	core->BindBackBuffer();
	core->ClearBackBuffer(0.0f, 0.0f, 0.0f, 1.0f);
	core->ApplyViewport(GraphicsCore::VirtualWidth, GraphicsCore::VirtualHeight);

	postProcessPass->Rendering(mainRenderTarget);
	if (captureRequested)
	{
		captureRequested = false;
		capturedPng.clear();
		capturedWidth = capturedHeight = 0;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		DirectX::ScratchImage image;
		if (core->GetSwapChain() &&
			SUCCEEDED(core->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()))) &&
			SUCCEEDED(DirectX::CaptureTexture(core->GetDevice(), context, backBuffer.Get(), image)))
		{
			const DirectX::Image* source = image.GetImage(0, 0, 0);
			DirectX::ScratchImage resized;
			if (source && source->width > static_cast<size_t>(captureMaxWidth))
			{
				const size_t height = (std::max)(size_t(1), source->height * captureMaxWidth / source->width);
				if (SUCCEEDED(DirectX::Resize(*source, captureMaxWidth, height, DirectX::TEX_FILTER_DEFAULT, resized)))
					source = resized.GetImage(0, 0, 0);
				else source = nullptr;
			}
			DirectX::Blob png;
			const HRESULT comStatus = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
			if (source && SUCCEEDED(DirectX::SaveToWICMemory(*source, DirectX::WIC_FLAGS_NONE,
				GUID_ContainerFormatPng, png)))
			{
				const auto* bytes = static_cast<const std::uint8_t*>(png.GetBufferPointer());
				capturedPng.assign(bytes, bytes + png.GetBufferSize());
				capturedWidth = static_cast<int>(source->width);
				capturedHeight = static_cast<int>(source->height);
			}
			if (SUCCEEDED(comStatus)) CoUninitialize();
		}
	}
	core->Present();
}

bool PixelGraphics::ForwardRender::RequestFrameCapture(int maxWidth)
{
	if (maxWidth < 320 || maxWidth > 1920) return false;
	captureMaxWidth = maxWidth;
	captureRequested = true;
	capturedPng.clear();
	capturedWidth = capturedHeight = 0;
	return true;
}

int PixelGraphics::ForwardRender::CopyFrameCapture(std::uint8_t* destination, int capacity, int* width, int* height)
{
	if (capturedPng.empty()) return 0;
	if (width) *width = capturedWidth;
	if (height) *height = capturedHeight;
	const int required = static_cast<int>(capturedPng.size());
	if (destination == nullptr || capacity == 0) return required;
	if (capacity < required) return -required;
	std::memcpy(destination, capturedPng.data(), capturedPng.size());
	capturedPng.clear();
	return required;
}
