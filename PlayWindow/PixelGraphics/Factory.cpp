#include "Factory.h"
#include "CBufferResources.h"
#include "InputLayout.h"
#include "RenderringData.h"
#include "VertexData.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <type_traits>
Factory::Factory()
{

}

Factory::~Factory()
{

}

void Factory::Initialize()
{
	mRasterizer_List.insert({ "Solid",CreateRasterizerState_Solid() });
	mRasterizer_List.insert({ "Wire",CreateRasterizerState_Wire() });
	mModelBufferList.insert({ "Axis", DebugAxis() });
	mModelBufferList.insert({ "Quad", DebugQuad() });
}

void Factory::Release()
{
	delete this;
}

DirectModel* Factory::DebugAxis()
{
	DebugVertex Debug[6];
	int Index[6] = { 0,1,2,3,4,5 };
	Debug[0].Pos = { 0,0,0 };
	Debug[0].Color = { 10,0,0 };

	Debug[1].Pos = { 10,0,0 };
	Debug[1].Color = { 10,0,0 };

	Debug[2].Pos = { 0,0,0 };
	Debug[2].Color = { 0,10,0 };

	Debug[3].Pos = { 0,10,0 };
	Debug[3].Color = { 0,10,0 };

	Debug[4].Pos = { 0,0,0 };
	Debug[4].Color = { 0,0,1 };

	Debug[5].Pos = { 0,0,10 };
	Debug[5].Color = { 0,0,1 };

	return CreateModelBuffer(Debug, 6, Index, 6);
}

DirectModel* Factory::DebugQuad()
{
	StaticVertex Debug[4];
	int Index[6] = { 0, 1, 2, 0, 2, 3 };

	Debug[0].Pos	= { -0.5f,0.5f,0 };
	Debug[0].UV		= { 0.0f,0.0f};

	Debug[1].Pos	= { 0.5f,0.5f,0 };
	Debug[1].UV		= { 1.0f,0.0f };

	Debug[2].Pos	= { 0.5f,-0.5f,0 };
	Debug[2].UV		= { 1.0f, 1.0f };

	Debug[3].Pos	= { -0.5f,-0.5f,0 };
	Debug[3].UV		= { 0.0f, 1.0f };

	return CreateModelBuffer(Debug, 4, Index, 6);
}



RenderingData* Factory::CreateRenderingData()
{
	int Size = (int)mRendering_List.size();
	for(int i = 0; i < Size; i++)
	{
		if(mRendering_List[i] == nullptr)
		{
			mRendering_List[i] = new RenderingData();
			return mRendering_List[i];
		}
	}

	RenderingData* mData = new RenderingData();
	mRendering_List.push_back(mData);
	return mData;
}

void Factory::DeleteRenderingData(RenderingData* Data)
{
	int Size = (int)mRendering_List.size();
	for (int i = 0; i < Size; i++)
	{
		if (mRendering_List[i] == Data)
		{
			delete Data;
			mRendering_List[i] = nullptr;
		}
	}
}

ObjectID Factory::CreateDebugModel(float* VertexList, int VertexSizeAll, int* IndexList, int IndexSize)
{
	int VertexSize  = VertexSizeAll / 3;
	int VertexIndex = 0;
	DebugVertex* Debug = new DebugVertex[VertexSize];
	for(int i =0; i < VertexSizeAll; i+= 3)
	{
		Debug[VertexIndex].Pos = { VertexList[i],VertexList[i + 1], VertexList[i + 2]};
		Debug[VertexIndex].Color = { 1.0f,1.0f,1.0f };
		VertexIndex++;
	}

	DirectModel* Model = CreateModelBuffer(Debug, VertexSize, IndexList, IndexSize);
	return SetModel(Model);
}

ObjectID Factory::CreateDebugModel(Vertex_Debug* VertexList, int VertexSize, int* IndexList, int IndexSize)
{
	DebugVertex* Debug = new DebugVertex[VertexSize];
	for(int i = 0; i < VertexSize;i++)
	{
		Debug[i].Pos.x = VertexList[i].Position[0];
		Debug[i].Pos.y = VertexList[i].Position[1];
		Debug[i].Pos.z = VertexList[i].Position[2];

		Debug[i].Color.x = VertexList[i].Color[0];
		Debug[i].Color.y = VertexList[i].Color[1];
		Debug[i].Color.z = VertexList[i].Color[2];
	}
	DirectModel* Model = CreateModelBuffer(Debug, VertexSize, IndexList, IndexSize);
	delete[] Debug;
	return SetModel(Model);
}

ID3D11RasterizerState* Factory::CreateRasterizerState_Solid()
{
	ID3D11RasterizerState* mSolid = nullptr;
	D3D11_RASTERIZER_DESC solidDesc;
	ZeroMemory(&solidDesc, sizeof(D3D11_RASTERIZER_DESC));
	solidDesc.FillMode = D3D11_FILL_SOLID;
	solidDesc.CullMode = D3D11_CULL_BACK;
	solidDesc.FrontCounterClockwise = false;
	solidDesc.DepthClipEnable = true;
	GraphicsCore::GetDevice()->CreateRasterizerState(&solidDesc, &mSolid);
	return mSolid;
}

ID3D11RasterizerState* Factory::CreateRasterizerState_Wire()
{
	ID3D11RasterizerState* mWire = nullptr;
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	GraphicsCore::GetDevice()->CreateRasterizerState(&wireframeDesc, &mWire);
	return mWire;
}

ObjectID Factory::CreateTextureResource(const char* filePath)
{
	int width, height, channels;
	// 1. 이미지를 CPU 메모리로 로드 (RGBA 강제)
	unsigned char* pixels = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels) {
		// 에러 처리: 파일이 없거나 포맷이 다를 때
		return -1;
	}

	// 2. Texture2D 설명 설정
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 일반적인 32비트 색상 포맷
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// 3. 초기 데이터 설정 (CPU 픽셀 -> GPU 초기화 데이터)
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pixels;
	initData.SysMemPitch = width * 4; // RGBA니까 너비 * 4바이트

	ID3D11Texture2D* pTexture = nullptr;
	
	GraphicsCore::GetDevice()->CreateTexture2D(&texDesc, &initData, &pTexture);
	// 4. Shader Resource View(SRV) 생성
	ID3D11ShaderResourceView* pSRV = nullptr;
	if (pTexture) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		GraphicsCore::GetDevice()->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
		pTexture->Release(); // SRV가 내부 참조를 가지므로 Texture 객체 자체는 해제 가능
	}

	stbi_image_free(pixels); // CPU 메모리 해제
	ObjectID id = std::hash<ID3D11ShaderResourceView*>{}(pSRV);

	mTexture_Map.insert({ id,pSRV });
	return id;
}
