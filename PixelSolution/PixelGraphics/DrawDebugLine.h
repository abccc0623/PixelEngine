#pragma once
#include"InputLayout.h"
struct RasterizerStateResources;
struct ShaderResources;
struct ID3D11SamplerState;
struct ID3D11Buffer;

class GraphicsEngine;
class DrawDebugLine
{
public:
	DrawDebugLine();
	~DrawDebugLine();

	void Initialize(GraphicsEngine* engine);
	void Set(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 color);
	void Draw();
private:

	ID3D11Buffer* CreateDynamicLineBuffer(int MaxVertexCount);
	ID3D11SamplerState* CreateSampler();
	
	ID3D11Buffer* linebuffer;
	RasterizerStateResources* rasterizerState;
	ShaderResources* shader;
	ID3D11SamplerState* sampler;

	std::vector<DebugVertex> lineList;
};

