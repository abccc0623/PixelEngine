#include "Factory.h"
#include "CBufferResources.h"
#include "InputLayout.h"
#include "RenderringData.h"
#include "VertexData.h"
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
	DebugVertex Debug[4];
	int Index[6] = { 0, 1, 2, 0, 2, 3 };

	Debug[0].Pos	= { -0.5f,0.5f,0 };
	Debug[0].Color	= { 1,0,0 };

	Debug[1].Pos	= { 0.5f,0.5f,0 };
	Debug[1].Color	= { 1,0,0 };

	Debug[2].Pos	= { 0.5f,-0.5f,0 };
	Debug[2].Color	= { 1,0,0 };

	Debug[3].Pos	= { -0.5f,-0.5f,0 };
	Debug[3].Color	= { 1,0,0 };

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

int Factory::CreateDebugModel(float* VertexList, int VertexSizeAll, int* IndexList, int IndexSize)
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

int Factory::CreateDebugModel(Vertex_Debug* VertexList, int VertexSize, int* IndexList, int IndexSize)
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
