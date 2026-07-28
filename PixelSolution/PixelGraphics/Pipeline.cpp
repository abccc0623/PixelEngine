#include "pch.h"
#include "Pipeline.h"

#include "GraphicsCore.h"
#include "MaterialFactory.h"
#include "ModelFactory.h"
#include "RasterizerStateFactory.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"
#include "TextureFactory.h"
#include "RenderringData.h"
#include "CBufferResources.h"

PixelGraphics::Pipeline::Pipeline(GraphicsCore* graphicsCore, ResourceManager* resources)
{
	core = graphicsCore;
	resourceManager = resources;
}

bool PixelGraphics::Pipeline::Initialize()
{
	textureFactory = resourceManager->GetFactory<TextureFactory>(ResourceType::TEXTURE);
	shaderFactory = resourceManager->GetFactory<ShaderFactory>(ResourceType::SHADER);
	rasterizerStateFactory = resourceManager->GetFactory<RasterizerStateFactory>(ResourceType::RASTERIZER_STATE);
	modelFactory = resourceManager->GetFactory<ModelFactory>(ResourceType::MODEL);
	materialFactory = resourceManager->GetFactory<MaterialFactory>(ResourceType::MATERIAL);
	PipelineInitialize();
	return true;
}

void PixelGraphics::Pipeline::Release()
{
	materialFactory = nullptr;
	modelFactory = nullptr;
	rasterizerStateFactory = nullptr;
	shaderFactory = nullptr;
	textureFactory = nullptr;
	resourceManager = nullptr;
	core = nullptr;
	PipelineRelease();
}


void PixelGraphics::Pipeline::Sort(std::vector<RenderingData>& data)
{
	std::sort(data.begin(), data.end(), [](const RenderingData& left, const RenderingData& right)
		{
			if (left.mash_key != right.mash_key)
			{
				return left.mash_key < right.mash_key;
			}
			if (left.material_key != right.material_key)
			{
				return left.material_key < right.material_key;
			}
			if (left.shader_key != right.shader_key)
			{
				return left.shader_key < right.shader_key;
			}
			return left.texture_key < right.texture_key;
		});
}

void PixelGraphics::Pipeline::SortUI(std::vector<RenderingData>& data)
{
	std::stable_sort(data.begin(), data.end(), [](const RenderingData& left, const RenderingData& right)
		{
			return left.sprite.Order < right.sprite.Order;
		});
}

void PixelGraphics::Pipeline::CameraSetting(RenderingData cameraData)
{
	constexpr float DefaultFovY = 0.3f * 3.1415926535f;
	constexpr float DefaultNearZ = 0.1f;
	constexpr float DefaultFarZ = 4000.0f;
	constexpr float MinClipDistance = 0.00001f;
	if (cameraData.camera.FarZ == 0.0f && cameraData.camera.FovY == 0.0f)
	{
		//기본값으로 셋팅
		cameraData.camera.Projection = ProjectionType::Perspective;
		cameraData.camera.FovY = DefaultFovY;
		cameraData.camera.NearZ = DefaultNearZ;
		cameraData.camera.FarZ = DefaultFarZ;
		cameraData.camera.ZoomLevel = 1.0f;
	}

	float width = cameraData.camera.ViewportWidth;
	float height = cameraData.camera.ViewportHeight;
	if (width <= 0.0f) width = static_cast<float>(core->GetClientWidth());
	if (height <= 0.0f) height = static_cast<float>(core->GetClientHeight());
	if (width <= 0.0f) width = 1.0f;
	if (height <= 0.0f) height = 1.0f;

	viewport.TopLeftX = cameraData.camera.ViewportX;
	viewport.TopLeftY = cameraData.camera.ViewportY;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// 2. View 행렬 계산 (World 행렬의 역행렬 적용)
	DirectX::SimpleMath::Matrix worldMatrix;
	memcpy(&worldMatrix, cameraData.World, sizeof(float) * 16);
	DirectX::SimpleMath::Matrix viewMatrix = worldMatrix;
	DirectX::SimpleMath::Matrix projMatrix;

	// 3. 매직 넘버 제거 및 카메라 컴포넌트 데이터 활용 (가정)
	// 실제 환경에서는 mData->camera.Fov, NearZ, FarZ 등을 가져와야 합니다.
	float fovY = cameraData.camera.FovY;
	float aspect = width / height;
	float nearZ = cameraData.camera.NearZ;
	float farZ = cameraData.camera.FarZ;

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

	if (cameraData.camera.Projection == ProjectionType::Perspective)
	{
		projMatrix = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
	}
	else // Orthographic
	{
		float zoomLevel = cameraData.camera.ZoomLevel; // 카메라 데이터에서 로드
		if (zoomLevel <= 0.0f) zoomLevel = 1.0f;
		float viewWidth = width / zoomLevel;
		float viewHeight = height / zoomLevel;
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
	auto context = core->GetDeviceContext();
	context->UpdateSubresource(cameraBuffer.Get(), 0, nullptr, &mCamBuffer, 0, 0);
	ID3D11Buffer* buffer = cameraBuffer.Get();
	context->VSSetConstantBuffers(0, 1, &(buffer));
	IsCameraSetting = true;
}

void PixelGraphics::Pipeline::ApplyViewport()
{
	if (!core || !core->GetDeviceContext())
	{
		return;
	}

	if (viewport.Width <= 0.0f || viewport.Height <= 0.0f)
	{
		core->ApplyViewport();
		return;
	}

	core->GetDeviceContext()->RSSetViewports(1, &viewport);
}

void PixelGraphics::Pipeline::SetBackgroundColor(float r, float g, float b)
{
	backgroundColor[0] = r;
	backgroundColor[1] = g;
	backgroundColor[2] = b;
	backgroundColor[3] = b;
}

DirectX::SimpleMath::Matrix& PixelGraphics::Pipeline::GetCameraView()
{
	if (IsCameraSetting == true)
	{
		return view;
	}
	OutputDebugStringA("[Graphics] Camera is not Setting!.\n");
	return view;
}
DirectX::SimpleMath::Matrix& PixelGraphics::Pipeline::GetCameraProjection()
{
	if (IsCameraSetting == true)
	{
		return proj;
	}
	OutputDebugStringA("[Graphics] Camera is not Setting!.\n");
	return proj;
}

DirectX::SimpleMath::Matrix PixelGraphics::Pipeline::GetUIProjection() const
{
	float width = static_cast<float>(core->GetClientWidth());
	float height = static_cast<float>(core->GetClientHeight());
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
