#pragma once
#include <windows.h>
#include <d3d11.h>

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
using ObjectID = size_t;
class GraphicsEngine;
struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ShaderResources;
struct DirectModel;
struct RenderingData;
struct ID3D11BlendState;
class GraphicsCore
{
public:
	GraphicsCore();
	virtual ~GraphicsCore();
private:
	static HWND mHwnd;
	static int ClientWidth;
	static int ClientHeight;
private: //다이렉트 자원들
	static ID3D11RenderTargetView*	mRenderTargetView;	//랜더 타겟
	static ID3D11DepthStencilView*	mDepthStencilView;	//뎁스 스텐실뷰
	static D3D11_VIEWPORT			mScreenViewport;	//뷰포트 
	static IDXGISwapChain*			mSwapChain;			//스왑체인
	static ID3D11Device*			mDevice;			//디바이스
	static ID3D11DeviceContext*		mDeviceContext;		//디바이스 컨텍스트
	static D3D_DRIVER_TYPE			mDriverType;		//드라이버 타입
private:
	static void GraphicsInitialize(HWND WindowHandle, int Width, int Height);
	static void GraphicsReSize(int Width, int Height);
	static void GraphicsRelease();
	static void BeginRender(float R = 0.0f, float G = 0.0f, float B = 0.0f, float A = 0.0f);
	static void EndRender();
	static void CreateBlend();
	friend GraphicsEngine;
protected:
	ObjectID SetModel(DirectModel* Model);
	static ID3D11BlendState* pAlphaBlendState;
	static std::unordered_map<ObjectID, std::vector<RenderingData*>> mRendering_List;
public:
	static ID3D11Device* GetDevice();
	static ID3D11DeviceContext* GetDeviceContext();
	static int GetClientWidth();
	static int GetClientHeight();
	static void Resize(int width, int height);
};

