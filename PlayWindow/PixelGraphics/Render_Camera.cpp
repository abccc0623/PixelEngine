#include "Render_Camera.h"
#include "CBufferResources.h"
#include "RenderringData.h"
Render_Camera::Render_Camera()
{

}

Render_Camera::~Render_Camera()
{

}

void Render_Camera::Binding(float* m)
{
	CameraBuffer mCamBuffer;
	ShaderResources* mResources = mShaderResources_List["Debug"];
	DirectX::SimpleMath::Matrix mWorld = DirectX::SimpleMath::Matrix::Identity;
	memcpy(&mWorld, m, sizeof(float) * 16);


	//시야각
	float mFovY = 0.3f * 3.1415926535f;
	//뷰 공간 X:Y 가로세로비율
	float mAspect = (float)GetClientWidth() / (float)GetClientHeight();
	//근접평면 까지의 거리 0보다 커야됨
	float mNearZ = 0.1f;
	//먼 평면 까지의 거리 0보다 커야됨
	float mFarZ = 4000.0f;

	float mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
	float mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f * mFovY);

	//원근 투영
	DirectX::SimpleMath::Matrix mProj = DirectX::XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);

	mCamBuffer.view = DirectX::XMMatrixTranspose(mWorld);
	mCamBuffer.proj = DirectX::XMMatrixTranspose(mProj);
	mCamBuffer.view_proj = DirectX::XMMatrixTranspose(mWorld * mProj);

	GetDeviceContext()->UpdateSubresource(mContextBuffer_List["CameraBuffer"], 0, nullptr, &mCamBuffer, 0, 0);
	GetDeviceContext()->VSSetConstantBuffers(0, 1, &mContextBuffer_List["CameraBuffer"]);
}

void Render_Camera::Binding(RenderingData* mData)
{
	CameraBuffer mCamBuffer;
	ShaderResources* mResources = mShaderResources_List["Debug"];
	DirectX::SimpleMath::Matrix mView = DirectX::SimpleMath::Matrix::Identity;
	memcpy(&mView, mData->World, sizeof(float) * 16);


	float mFovY = 0.3f * 3.1415926535f;
	float mAspect = (float)GetClientWidth() / (float)GetClientHeight();
	float mNearZ = 0.1f;
	float mFarZ = 4000.0f;
	float mNearWindowHeight = 2.0f * mNearZ * tanf(0.5f * mFovY);
	float mFarWindowHeight = 2.0f * mFarZ * tanf(0.5f * mFovY);
	
	DirectX::SimpleMath::Matrix mProj;
	
	//{
	//	mProj = DirectX::XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	//}

	{
		mNearZ = 1.0f; // 직교 투영은 0부터 시작해도 무방합니다.
		mFarZ = 4000.0f;
		mProj = DirectX::XMMatrixOrthographicLH(GetClientWidth(), GetClientHeight(), mNearZ, mFarZ);
	}



	mCamBuffer.view			= DirectX::XMMatrixTranspose(mView);
	mCamBuffer.proj			= DirectX::XMMatrixTranspose(mProj);
	mCamBuffer.view_proj	= DirectX::XMMatrixTranspose(mView * mProj);
	GetDeviceContext()->UpdateSubresource(mContextBuffer_List["CameraBuffer"], 0, nullptr, &mCamBuffer, 0, 0);
	GetDeviceContext()->VSSetConstantBuffers(0, 1, &mContextBuffer_List["CameraBuffer"]);
}

