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
	core->CreateRenderTarget(1920, 1080, mainRenderTarget);
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
	core->ClearBackBuffer(1.0f, 0.0f, 1.0f, 1.0f);
	core->ApplyViewport();

	postProcessPass->Rendering(mainRenderTarget);
	core->Present();
}
