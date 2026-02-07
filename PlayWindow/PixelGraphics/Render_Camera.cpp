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
	DirectX::SimpleMath::Matrix View =
	{
		m[0], m[1], m[2], m[3],
		m[4], m[5], m[6], m[7],
		m[8], m[9], m[10],m[11],
		m[12],m[13],m[14],m[15],
	};

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

	mCamBuffer.view = DirectX::XMMatrixTranspose(View);
	mCamBuffer.proj = DirectX::XMMatrixTranspose(mProj);
	mCamBuffer.view_proj = DirectX::XMMatrixTranspose(View * mProj);

	GetDeviceContext()->UpdateSubresource(mContextBuffer_List["CameraBuffer"], 0, nullptr, &mCamBuffer, 0, 0);
	GetDeviceContext()->VSSetConstantBuffers(0, 1, &mContextBuffer_List["CameraBuffer"]);
}

void Render_Camera::Binding(RenderingData* mData)
{
	CameraBuffer mCamBuffer;
	ShaderResources* mResources = mShaderResources_List["Debug"];
	DirectX::SimpleMath::Matrix View =
	{
		mData->World[0], mData->World[1], mData->World[2], mData->World[3],
		mData->World[4], mData->World[5], mData->World[6], mData->World[7],
		mData->World[8], mData->World[9], mData->World[10],mData->World[11],
		mData->World[12],mData->World[13],mData->World[14],mData->World[15],
	};

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
	DirectX::SimpleMath::Matrix mProj = DirectX::XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);

	mCamBuffer.view			= DirectX::XMMatrixTranspose(View);
	mCamBuffer.proj			= DirectX::XMMatrixTranspose(mProj);
	mCamBuffer.view_proj	= DirectX::XMMatrixTranspose(View * mProj);

	GetDeviceContext()->UpdateSubresource(mContextBuffer_List["CameraBuffer"], 0, nullptr, &mCamBuffer, 0, 0);
	GetDeviceContext()->VSSetConstantBuffers(0, 1, &mContextBuffer_List["CameraBuffer"]);
}

