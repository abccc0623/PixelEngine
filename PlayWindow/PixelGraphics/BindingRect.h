#pragma once

struct RenderingData;
struct BufferResources;
struct DirectModel;
struct RasterizerStateResources;
struct ShaderResources;
struct TextureResources;
struct ID3D11SamplerState;

#include "BindingClass.h"
class BindingRect : public BindingClass
{
public:
	BindingRect();
	~BindingRect();
	void Initialize() override;
	void Binding(RenderingData* mData, Handle64 prev) override;

private:
	ID3D11SamplerState* CreateSampler();
	BufferResources* targetBuffer;
	DirectModel* quadModel;
	RasterizerStateResources* rasterizerState;
	ShaderResources* shader;
	TextureResources* texture;
	ID3D11SamplerState* sampler;
};

