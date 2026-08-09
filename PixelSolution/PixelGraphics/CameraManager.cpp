#include "pch.h"
#include "CameraManager.h"
#include "RenderringData.h"
#include "GraphicsCore.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"
#include "CBufferResources.h"
#include "PixelResources.h"

#undef max
#undef min
#include <algorithm>
PixelGraphics::CameraManager::CameraManager(GraphicsCore* core, ResourceManager* resourceManager)
{
	this->core = core;
	this->resourceManager = resourceManager;
	shaderFactory = resourceManager->GetFactory<ShaderFactory>(ResourceType::SHADER);
	cameraBufferResource = shaderFactory->GetBuffer("CameraBuffer");
}

PixelGraphics::CameraManager::~CameraManager()
{
	this->core = nullptr;
	this->resourceManager = nullptr;
}

void PixelGraphics::CameraManager::Setting(RenderingData& r)
{
	constexpr float DefaultFovY = 0.3f * 3.1415926535f;
	constexpr float DefaultNearZ = 0.1f;
	constexpr float DefaultFarZ = 4000.0f;
	constexpr float MinClipDistance = 0.00001f;
	if (r.camera.FarZ == 0.0f && r.camera.FovY == 0.0f)
	{
		//기본값으로 셋팅
		r.camera.Projection = ProjectionType::Perspective;
		r.camera.FovY = DefaultFovY;
		r.camera.NearZ = DefaultNearZ;
		r.camera.FarZ = DefaultFarZ;
		r.camera.ZoomLevel = 1.0f;
	}
	ViewPortSetting(r);

	// 2. View 행렬 계산 (World 행렬의 역행렬 적용)
	DirectX::SimpleMath::Matrix worldMatrix;
	memcpy(&worldMatrix, r.World, sizeof(float) * 16);
	DirectX::SimpleMath::Matrix viewMatrix = worldMatrix;
	DirectX::SimpleMath::Matrix projMatrix;

	// 3. 매직 넘버 제거 및 카메라 컴포넌트 데이터 활용 (가정)
	// 실제 환경에서는 mData->camera.Fov, NearZ, FarZ 등을 가져와야 합니다.
	float viewportWidth = r.camera.ViewportWidth;
	float viewportHeight = r.camera.ViewportHeight;
	if (!std::isfinite(viewportWidth) || viewportWidth <= 0.0f)
	{
		viewportWidth = static_cast<float>(GraphicsCore::VirtualWidth);
	}
	if (!std::isfinite(viewportHeight) || viewportHeight <= 0.0f)
	{
		viewportHeight = static_cast<float>(GraphicsCore::VirtualHeight);
	}
	if (viewportWidth <= 0.0f)
	{
		viewportWidth = 1.0f;
	}
	if (viewportHeight <= 0.0f)
	{
		viewportHeight = 1.0f;
	}

	float fovY = r.camera.FovY;
	float aspect = viewportWidth / viewportHeight;
	float nearZ = r.camera.NearZ;
	float farZ = r.camera.FarZ;

	if (!std::isfinite(fovY) || fovY <= 0.0f)
	{
		fovY = DefaultFovY;
	}
	if (!std::isfinite(nearZ) || nearZ <= 0.0f)
	{
		nearZ = DefaultNearZ;
	}
	if (!std::isfinite(farZ) || farZ <= nearZ + MinClipDistance)
	{
		farZ = DefaultFarZ;
		if (farZ <= nearZ + MinClipDistance)
		{
			farZ = nearZ + 1.0f;
		}
	}

	if (r.camera.Projection == ProjectionType::Perspective)
	{
		projMatrix = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
	}
	else // Orthographic
	{
		float zoomLevel = r.camera.ZoomLevel; // 카메라 데이터에서 로드
		if (zoomLevel <= 0.0f) zoomLevel = 1.0f;
		float viewWidth = viewportWidth / zoomLevel;
		float viewHeight = viewportHeight / zoomLevel;
		projMatrix = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);
	}

	// 5. 상수 버퍼 데이터 패킹
	CameraBuffer mCamBuffer;
	mCamBuffer.view = viewMatrix.Transpose();
	mCamBuffer.proj = projMatrix.Transpose();
	mCamBuffer.view_proj = (viewMatrix * projMatrix).Transpose();
	this->view = viewMatrix;
	this->proj = projMatrix;

	// 6. TODO: 추후 UpdateSubresource 대신 Map/Unmap (WRITE_DISCARD) 방식으로 리팩토링 고려

	auto buffer = cameraBufferResource->buffer.Get();
	auto context = core->GetDeviceContext();
	context->UpdateSubresource(buffer, 0, nullptr, &mCamBuffer, 0, 0);
	context->VSSetConstantBuffers(0, 1, &(buffer));
}

DirectX::SimpleMath::Matrix PixelGraphics::CameraManager::GetView()
{
	return view;
}

DirectX::SimpleMath::Matrix PixelGraphics::CameraManager::GetProj()
{
	return proj;
}

DirectX::SimpleMath::Matrix PixelGraphics::CameraManager::GetProjUI()
{
	float width = static_cast<float>(GraphicsCore::VirtualWidth);
	float height = static_cast<float>(GraphicsCore::VirtualHeight);
	if (width <= 0.0f)
	{
		width = 1.0f;
	}
	if (height <= 0.0f)
	{
		height = 1.0f;
	}

	return DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f,
		width,
		height,
		0.0f,
		0.0f,
		1.0f);
}

void PixelGraphics::CameraManager::BindViewPort()
{
	core->GetDeviceContext()->RSSetViewports(1, &targetViewport);
}

void PixelGraphics::CameraManager::ViewPortSetting(RenderingData& r)
{
	float targetWidth = r.camera.ViewportWidth;
	float targetHeight = r.camera.ViewportHeight;

	if (targetWidth <= 0.0f)
		targetWidth = static_cast<float>(GraphicsCore::VirtualWidth);

	if (targetHeight <= 0.0f)
		targetHeight = static_cast<float>(GraphicsCore::VirtualHeight);

	const float viewportX = std::max(0.0f, r.camera.ViewportX);
	const float viewportY = std::max(0.0f, r.camera.ViewportY);

	targetViewport.TopLeftX = viewportX;
	targetViewport.TopLeftY = viewportY;
	targetViewport.Width = targetWidth;
	targetViewport.Height = targetHeight;
	targetViewport.MinDepth = 0.0f;
	targetViewport.MaxDepth = 1.0f;
}
