#include "pch.h"
#include "BindingCamera.h"
#include "GraphicsEngine.h"
#include "CBufferResources.h"
#include "RenderringData.h"
#include "PixelResources.h"
#include "GraphicsCore.h"
BindingCamera::BindingCamera()
{

}

BindingCamera::~BindingCamera()
{

}


void BindingCamera::Initialize()
{
	cameraBuffer = engine->Get<BufferResources>("CameraBuffer");
}

void BindingCamera::Binding(RenderingData* mData, Handle64 prev)
{
	if (mData->camera.FarZ == 0.0f && mData->camera.FovY == 0.0f)
	{
		//기본값으로 셋팅
		mData->camera.Projection = ProjectionType::Perspective;
		mData->camera.FovY = 0.3f * 3.1415926535f;
		mData->camera.NearZ = 0.1f;
		mData->camera.FarZ = 4000.0f;
		mData->camera.ZoomLevel = 1.0f;
	}

	// 1. 방어적 프로그래밍 (Zero Divide 방지)
	float width = (float)GraphicsCore::GetClientWidth();
	float height = (float)GraphicsCore::GetClientHeight();
	if (height <= 0.0f) height = 1.0f;

	// 2. View 행렬 계산 (World 행렬의 역행렬 적용)
	DirectX::SimpleMath::Matrix worldMatrix;
	memcpy(&worldMatrix, mData->World, sizeof(float) * 16);
	DirectX::SimpleMath::Matrix viewMatrix = worldMatrix.Invert();

	DirectX::SimpleMath::Matrix projMatrix;

	// 3. 매직 넘버 제거 및 카메라 컴포넌트 데이터 활용 (가정)
	// 실제 환경에서는 mData->camera.Fov, NearZ, FarZ 등을 가져와야 합니다.
	float fovY = mData->camera.FovY;
	float aspect = width / height;
	float nearZ = mData->camera.NearZ;
	float farZ = mData->camera.FarZ;

	if (mData->camera.Projection == ProjectionType::Perspective)
	{
		projMatrix = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
	}
	else // Orthographic
	{
		float zoomLevel = mData->camera.ZoomLevel; // 카메라 데이터에서 로드
		if (zoomLevel <= 0.0f) zoomLevel = 1.0f;

		float viewWidth = width / zoomLevel;
		float viewHeight = height / zoomLevel;

		projMatrix = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, nearZ, farZ);
	}

	// 4. 멀티스레딩 및 구조적 안전성을 위해 로컬 변수 우선 활용
	// (추후 GraphicsCore::mView 의존성을 완전히 분리하는 것을 권장합니다)
	GraphicsCore::mView = viewMatrix;
	GraphicsCore::mProj = projMatrix;

	// 5. 상수 버퍼 데이터 패킹
	CameraBuffer mCamBuffer;
	mCamBuffer.view = viewMatrix.Transpose();
	mCamBuffer.proj = projMatrix.Transpose();
	mCamBuffer.view_proj = (viewMatrix * projMatrix).Transpose();

	// 6. TODO: 추후 UpdateSubresource 대신 Map/Unmap (WRITE_DISCARD) 방식으로 리팩토링 고려
	auto context = GraphicsCore::GetDeviceContext();
	context->UpdateSubresource(cameraBuffer->buffer, 0, nullptr, &mCamBuffer, 0, 0);
	context->VSSetConstantBuffers(0, 1, &(cameraBuffer->buffer));
}


