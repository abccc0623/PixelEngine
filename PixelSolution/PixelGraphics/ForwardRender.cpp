#include "pch.h"
#include "ForwardRender.h"
#include <cstring>
#include "GraphicsCore.h"
#include "RenderringData.h"
#include "ResourceManager.h"
#include "CameraManager.h"
#include "ScenePass.h"
#include "UIPass.h"
#include "PostProcessPass.h"

using namespace DirectX::SimpleMath;
PixelGraphics::ForwardRender::ForwardRender(GraphicsCore* graphicsCore, ResourceManager* resources) : Pipeline(graphicsCore, resources)
{
	cameraManager = new CameraManager(graphicsCore, resources);
	passList.push_back(new ScenePass(graphicsCore, resources, cameraManager));
	passList.push_back(new UIPass(graphicsCore, resources, cameraManager));
	postProcessPass = new PostProcessPass(graphicsCore, resources, cameraManager);
}
void PixelGraphics::ForwardRender::PipelineInitialize()
{
	core->CreateRenderTarget(core->GetClientWidth(), core->GetClientHeight(), mainRenderTarget);
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
	else if (renderingData.renderType == RENDER_TYPE::TEXT)
	{

	}
	else if (renderingData.passType == PASS_TYPE::SCENE)
	{
		passList[0]->SetRenderingData(renderingData);

	}
	else if (renderingData.passType == PASS_TYPE::UI)
	{
		passList[1]->SetRenderingData(renderingData);
	}
	else
	{

	}
}


void PixelGraphics::ForwardRender::Rendering()
{
	ID3D11DeviceContext* context = core->GetDeviceContext();
	ID3D11RenderTargetView* sceneRTV = mainRenderTarget.renderTargetView.Get();

	context->OMSetRenderTargets(1, &sceneRTV, nullptr);
	//D3D11_VIEWPORT viewport{};
	//viewport.Width = static_cast<float>(mainRenderTarget.width);
	//viewport.Height = static_cast<float>(mainRenderTarget.height);
	//viewport.MinDepth = 0.0f;
	//viewport.MaxDepth = 1.0f;
	//context->RSSetViewports(1, &viewport);
	cameraManager->BindViewPort();

	context->ClearRenderTargetView(sceneRTV, backgroundColor);
	for (auto& pass : passList)
	{
		pass->Rendering(mainRenderTarget);
	}

	//렌더 타깃 출력 연결 해제
	//context->OMSetRenderTargets(0, nullptr, nullptr);
	core->BindBackBuffer();
	core->ClearBackBuffer(1.0f, 0.0f, 1.0f, 1.0f);
	core->ApplyViewport();

	postProcessPass->Rendering(mainRenderTarget);
	core->Present();
}
