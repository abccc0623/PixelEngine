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
    float4x4    world;
    float4x4    wvp;
    float4x4    TexMatrix;
};

Texture2D MainTexture   : register(t0);
SamplerState SampleType : register(s0);

struct VertexInputType
{
    float3 position		: POSITION;
    float2 UV			: TEXCOORD;
    float3 NormalL		: NORMAL;
    float3 TangentL		: TANGENT;
};

struct PixelInputType
{
    float4 posH			: SV_POSITION;
    float2 UV           : TEXCOORD;
    float3 NormalW		: NORMAL;
    float3 TangentW		: TANGENT;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    //월드변환
    //float4 worldPos = mul(float4(input.position, 1.0f), wvp);
    output.posH = mul(float4(input.position, 1.0f), wvp);
    //uv값
    //output.UV = mul(TexMatrix,float4(input.UV, 0.0f, 1.0f)).xy;
    output.UV = mul(float4(input.UV, 0.0f, 1.0f), TexMatrix).xy;
    //노말
    //output.NormalW = mul(input.NormalL, (float3x3) view_proj);
    //탄젠트
    //output.TangentW = mul(input.TangentL, (float3x3) world);
 
    return output;
}