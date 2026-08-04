//카메라 관련 버퍼
cbuffer CameraBuffer : register(b0)
{
    float4x4 view;
    float4x4 proj;
    float4x4 view_proj;
};

//오브젝트 관련 버퍼
cbuffer ObjectBuffer : register(b1)
{
    float4x4 world;
    float4x4 wvp;
    float4x4 TexMatrix;
    float4 Color;
};

Texture2D MainTexture : register(t0);
SamplerState SampleType : register(s0);

struct VertexInputType
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct PixelInputType
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    output.pos = mul(float4(input.pos, 1.0f), wvp);
    output.uv = mul(float4(input.uv, 0.0f, 1.0f), TexMatrix).xy;
    return output;
}