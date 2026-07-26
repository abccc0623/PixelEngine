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
	if (!resources)
	{
		return;
	}

	textureFactory = resources->GetFactory<TextureFactory>(ResourceType::TEXTURE);
	shaderFactory = resources->GetFactory<ShaderFactory>(ResourceType::SHADER);
	rasterizerStateFactory = resources->GetFactory<RasterizerStateFactory>(ResourceType::RASTERIZER_STATE);
	modelFactory = resources->GetFactory<ModelFactory>(ResourceType::MODEL);
	materialFactory = resources->GetFactory<MaterialFactory>(ResourceType::MATERIAL);
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
}

void PixelGraphics::Pipeline::SetCamera(RenderingData& cameraData)
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

	// 1. 방어적 프로그래밍 (Zero Divide 방지)
	float width = (float)core->GetClientWidth();
	float height = (float)core->GetClientHeight();
	if (height <= 0.0f) height = 1.0f;

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

	// 4. 멀티스레딩 및 구조적 안전성을 위해 로컬 변수 우선 활용
	// (추후 GraphicsCore::mView 의존성을 완전히 분리하는 것을 권장합니다)


	// 5. 상수 버퍼 데이터 패킹
	CameraBuffer mCamBuffer;
	mCamBuffer.view = viewMatrix.Transpose();
	mCamBuffer.proj = projMatrix.Transpose();
	mCamBuffer.view_proj = (viewMatrix * projMatrix).Transpose();
	this->view = viewMatrix;
	this->projection = projMatrix;

	// 6. TODO: 추후 UpdateSubresource 대신 Map/Unmap (WRITE_DISCARD) 방식으로 리팩토링 고려
	auto context = core->GetDeviceContext();
	context->UpdateSubresource(cameraBuffer.Get(), 0, nullptr, &mCamBuffer, 0, 0);
	ID3D11Buffer* buffer = objectBuffer.Get();
	context->VSSetConstantBuffers(0, 1, &(buffer));
}

DirectX::SimpleMath::Matrix& PixelGraphics::Pipeline::GetView()
{
	return view;
}

DirectX::SimpleMath::Matrix& PixelGraphics::Pipeline::GetProjection()
{
	return projection;
}
