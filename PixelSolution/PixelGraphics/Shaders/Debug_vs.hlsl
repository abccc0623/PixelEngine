
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
    float4x4 TexMatrix;
};

struct VertexInputType
{
	float3 PosL			: POSITION;
	float3 Color		: COLOR;
};

struct PixelInputType
{
    float4 posH     : SV_POSITION;
    float3 Tex      : COLOR;
};


PixelInputType main(VertexInputType input)
{
   PixelInputType output;

    float4 worldPos = mul(float4(input.PosL, 1.0f), world);

    output.posH = mul(worldPos, view_proj);
    output.Tex = input.Color;
    return output;
}