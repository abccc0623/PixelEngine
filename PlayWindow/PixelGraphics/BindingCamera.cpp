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
	CameraBuffer mCamBuffer;
	DirectX::SimpleMath::Matrix mView = DirectX::SimpleMath::Matrix::Identity;
	memcpy(&mView, mData->World, sizeof(float) * 16);
	
	
	float mFovY = 0.3f * 3.1415926535f;
	float mAspect = (float)GraphicsCore::GetClientWidth() / (float)GraphicsCore::GetClientHeight();
	float mNearZ = 0.1f;
	float mFarZ = 4000.0f;
	float mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
	float mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f * mFovY);
	
	DirectX::SimpleMath::Matrix mProj;
	
	//mProj = DirectX::XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);

	if (mData->camera.Projection == ProjectionType::Perspective)
	{
		mProj = DirectX::XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	}
	else
	{
		float width = (float)GraphicsCore::GetClientWidth();
		float height = (float)GraphicsCore::GetClientHeight();

		float zoomLevel = 100.0f;

		float viewWidth = width / zoomLevel;
		float viewHeight = height / zoomLevel;

		mNearZ = 0.1f; // 너무 1.0f이면 카메라 바로 앞 객체가 잘릴 수 있으니 살짝 낮춤
		mFarZ = 4000.0f;
		mProj = DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, mNearZ, mFarZ);
	}
	
	mCamBuffer.view			= DirectX::XMMatrixTranspose(mView);
	mCamBuffer.proj			= DirectX::XMMatrixTranspose(mProj);
	mCamBuffer.view_proj	= DirectX::XMMatrixTranspose(mView * mProj);
	GraphicsCore::GetDeviceContext()->UpdateSubresource(cameraBuffer->buffer, 0, nullptr, &mCamBuffer, 0, 0);
	GraphicsCore::GetDeviceContext()->VSSetConstantBuffers(0, 1, &(cameraBuffer->buffer));
}	


