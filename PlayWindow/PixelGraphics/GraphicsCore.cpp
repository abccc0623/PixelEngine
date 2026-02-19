#include "GraphicsCore.h"
#include "PixelGraphicsLib.h"
#include "RenderringData.h"


ID3D11Device*			GraphicsCore::mDevice			= nullptr;
ID3D11DeviceContext*	GraphicsCore::mDeviceContext	= nullptr;
IDXGISwapChain*			GraphicsCore::mSwapChain		= nullptr;
ID3D11RenderTargetView* GraphicsCore::mRenderTargetView = nullptr;
ID3D11DepthStencilView* GraphicsCore::mDepthStencilView = nullptr;
D3D11_VIEWPORT			GraphicsCore::mScreenViewport;
D3D_DRIVER_TYPE			GraphicsCore::mDriverType;
ID3D11BlendState*		GraphicsCore::pAlphaBlendState  = nullptr;
int GraphicsCore::ClientHeight = 0;
int GraphicsCore::ClientWidth  = 0;
HWND GraphicsCore::mHwnd;

GraphicsCore::GraphicsCore()
{

}

GraphicsCore::~GraphicsCore()
{
	
}

void GraphicsCore::GraphicsReSize(int Width, int Height)
{

}

void GraphicsCore::GraphicsInitialize(HWND WindowHandle, int Width, int Height)
{
	ClientWidth		= Width;
	ClientHeight	= Height;
	mHwnd			= WindowHandle;

	UINT createDeviceFlags = 0;
	HRESULT hr;

	/**  다이렉트를 디버그모드로 설정한다  */
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	/** 드라이버 타입 */
	D3D_DRIVER_TYPE drierTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,	//100프로 GPU를 사용하는것
		D3D_DRIVER_TYPE_WARP,		//GPU는 없지만 CPU가 최신것일떄
		D3D_DRIVER_TYPE_REFERENCE	//소프트웨어 디바이스
	};

	/** 드라이버 타입은 3가지로 설정 */
	UINT NumDirverTyps = ARRAYSIZE(drierTypes);


	/**레벨 설정 (이것은 GPU가 정한다 다만 선택지를 줄뿐) */
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,	//최우선으로 생각하는 레벨 안됀다면 다음꺼
		D3D_FEATURE_LEVEL_10_1,	//그다음 레벨 안된다면 다음 꺼
		D3D_FEATURE_LEVEL_10_0, //그다음 레벨 안된다면 다음 꺼
	};

	/** 레벨 타입을 3가지로 설정*/
	UINT NumFeatureLevels = ARRAYSIZE(featureLevels);


	/** 스왑체인 구조체 설정
	(더블 버퍼링이라 생각하면 편안...)
	더블 버퍼링에 관련된것을 구조체로 뺀것
	*/
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = ClientWidth;
	sd.BufferDesc.Height = ClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = mHwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE; //윈도우 모드 또는 풀스크린 모드
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = 0; //디버깅 모드일때만 Direct11 디버그로 실행


	/**
	넘겨받은 구조체로 스왑체인을 생성
	for문을 도는 이유는 위에서 드라이버 타입을 3가지로 설정했는데
	해당 하드웨어가 해당타입이 아니면 다음 타입을 체크하기 위해서
	*/
	UINT driverTypeIndex = 0;
	for (driverTypeIndex; driverTypeIndex < 1; driverTypeIndex++)
	{
		mDriverType = drierTypes[driverTypeIndex];


		//스왑 체인 생성과정에 매개변수로 디바이스와, 디바이스컨텍스트를 매개변수로 사용하는데
		//이것은 원래 스왑체인과 ,디바이스가 따로생성해 합쳐주는데 편하게 사용하기위해 한번에 생성
		hr = D3D11CreateDeviceAndSwapChain
		(
			NULL, mDriverType, NULL, 0, featureLevels, 1,
			D3D11_SDK_VERSION, &sd, &mSwapChain, &mDevice, NULL, &mDeviceContext
		);


		//성공했다면 이대로 생성
		if (SUCCEEDED(hr)) { break; }
	}


	//랜더타겟 뷰 생성
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr)) { return; }


	hr = mDevice->CreateRenderTargetView(pBackBuffer, NULL, &mRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr)) { return; }


	//뎁스 버퍼 생성
	ID3D11Texture2D* mDepthStencilBuffer = nullptr;
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = ClientWidth;								//텍스쳐의 너비
	depthStencilDesc.Height = ClientHeight;								//텍스쳐의 높이 
	depthStencilDesc.MipLevels = 1;										//밉맵수준의 개수
	depthStencilDesc.ArraySize = 1;										//택스처 배열의 텍스처 개수*깊이*스텐실 버퍼의 경우 텍스처 하나만필요
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;								//SampleDesc = 다중표본 개수와 품질수듄을 서술하는 구조체
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;						//텍스처의 용도를 뜻하는 필드
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;				//자원을 파이프라인에 어떤식으로 묶을것인지
	depthStencilDesc.CPUAccessFlags = 0;								//CPU가 자원을 접근하는 방식을 결정하는 플래그를 지정
	depthStencilDesc.MiscFlags = 0;
	mDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer);
	hr = mDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);
	if (FAILED(hr)) { return; }

	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	//뷰포트 생성
	mScreenViewport.Width = (float)ClientWidth;
	mScreenViewport.Height = (float)ClientHeight;
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mDeviceContext->RSSetViewports(1, &mScreenViewport);
	CreateBlend();
}

void GraphicsCore::GraphicsRelease()
{
	/** Direct자원 삭제 */
	pAlphaBlendState->Release();
	mDepthStencilView->Release();
	mRenderTargetView->Release();
	mSwapChain->Release();
	mDeviceContext->Release();
	mDevice->Release();


	/** 삭제시 메모리값 초기화 */
	mRenderTargetView	= nullptr;
	mDepthStencilView	= nullptr;
	mSwapChain			= nullptr;
	mDevice				= nullptr;
	mDeviceContext		= nullptr;
}

void GraphicsCore::BeginRender(float R, float G, float B, float A)
{
	float DeepDarkGray[4] = { R, G, B, A };

	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	mDeviceContext->ClearRenderTargetView(mRenderTargetView, DeepDarkGray);
	mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	mDeviceContext->RSSetViewports(1, &mScreenViewport);
}

void GraphicsCore::EndRender()
{
	mSwapChain->Present(0, 0);
}

void GraphicsCore::CreateBlend()
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE; // 모든 RenderTarget에 동일하게 적용

	// 첫 번째 렌더 타겟 설정
	blendDesc.RenderTarget[0].BlendEnable = TRUE; // 블렌딩 활성화!
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;      // 소스에 알파 곱함
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 배경에 (1-알파) 곱함
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;          // 둘을 더함

	// 알파 채널 자체의 연산 설정
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pAlphaBlendState = nullptr;
	mDevice->CreateBlendState(&blendDesc, &pAlphaBlendState);
}

ID3D11DeviceContext* GraphicsCore::GetDeviceContext()
{
	if (mDeviceContext == nullptr) { return nullptr; }
	return mDeviceContext;
}

ID3D11Device* GraphicsCore::GetDevice()
{
	if (mDevice == nullptr) { return nullptr; }
	return mDevice;
}

int GraphicsCore::GetClientWidth()
{
	return ClientWidth;
}

int GraphicsCore::GetClientHeight()
{
	return ClientHeight;
}

void GraphicsCore::Resize(int width, int height)
{
	ClientWidth = width;
	ClientHeight = height;

	if (!mDevice || !mSwapChain) return;

	ClientWidth = width;
	ClientHeight = height;

	// 1. 기존에 연결된 뷰들을 모두 해제해야 합니다. (핵심!)
	mDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	if (mRenderTargetView) mRenderTargetView->Release();
	if (mDepthStencilView) mDepthStencilView->Release();

	// 2. 스왑 체인의 버퍼 크기를 변경합니다.
	// DXGI_SWAP_CHAIN_DESC에 설정했던 포맷과 동일하게 맞춰줍니다.
	HRESULT hr = mSwapChain->ResizeBuffers(2, ClientWidth, ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr)) { /* 에러 처리 */ return; }

	// 3. 변경된 백버퍼로부터 렌더 타겟 뷰를 다시 생성합니다.
	ID3D11Texture2D* pBackBuffer = nullptr;
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	mDevice->CreateRenderTargetView(pBackBuffer, NULL, &mRenderTargetView);
	pBackBuffer->Release();

	// 4. 뎁스 스텐실 버퍼와 뷰도 새로운 크기로 다시 생성합니다.
	// (Initialize에 있던 뎁스 버퍼 생성 로직을 여기에도 적용)
	D3D11_TEXTURE2D_DESC depthDesc;
	// ... (Initialize의 설정과 동일하게 하되 Width, Height만 새 값으로)
	depthDesc.Width = ClientWidth;
	depthDesc.Height = ClientHeight;
	// ... 
	mDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	ID3D11Texture2D* mDepthStencilBuffer = nullptr;
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = ClientWidth;								//텍스쳐의 너비
	depthStencilDesc.Height = ClientHeight;								//텍스쳐의 높이 
	depthStencilDesc.MipLevels = 1;										//밉맵수준의 개수
	depthStencilDesc.ArraySize = 1;										//택스처 배열의 텍스처 개수*깊이*스텐실 버퍼의 경우 텍스처 하나만필요
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;								//SampleDesc = 다중표본 개수와 품질수듄을 서술하는 구조체
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;						//텍스처의 용도를 뜻하는 필드
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;				//자원을 파이프라인에 어떤식으로 묶을것인지
	depthStencilDesc.CPUAccessFlags = 0;								//CPU가 자원을 접근하는 방식을 결정하는 플래그를 지정
	depthStencilDesc.MiscFlags = 0;
	mDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer);
	hr = mDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);
	if (FAILED(hr)) { return; }
	mDepthStencilBuffer->Release();

	// 5. 뷰포트 설정
	mScreenViewport.Width = (float)ClientWidth;
	mScreenViewport.Height = (float)ClientHeight;
	mDeviceContext->RSSetViewports(1, &mScreenViewport);

	// 6. 타겟 다시 바인딩
	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
}


