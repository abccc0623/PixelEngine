#include "MainHeader.hlsli"

struct VertexInputType
{
	float3  PosL        : POSITION;
	float2  UV			: TEXCOORD;
	float3  NormalL     : NORMAL;
	float3  TangentL    : TANGENT;
	float4  Weights     : WEIGHTS;
	uint4   BoneIndices : BONEINDICES;
};

struct PixelInputType
{
	float4  PosH		: SV_POSITION;
	float3  PosL		: POSITION;
	float2  UV			: TEXCOORD;
	float3  NormalL		: NORMAL;
	float3  TangentW	: TEXCOORD2;
};

PixelInputType main(VertexInputType input)
{
	PixelInputType output;

    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weights[0] = input.Weights.x;
    weights[1] = input.Weights.y;
    weights[2] = input.Weights.z;
    weights[3] = 1.0f - weights[0] - weights[1] - weights[2];
	
    //weights[3] = input.Weights.w;
	
    float3 posL     = float3(0.0f, 0.0f, 0.0f);
    float3 normalL  = float3(0.0f, 0.0f, 0.0f);
	
    for (int i = 0; i < 4; i++)
    {
        posL += weights[i] * mul(float4(input.PosL, 1.0f), BoneOffset[input.BoneIndices[i]]).xyz;
        normalL += weights[i] * mul(input.NormalL, (float3x3) BoneOffset[input.BoneIndices[i]]).xyz;
    }
   // output.PosH = mul(float4(posL, 1.0f), viewproj);
	
    output.PosL = mul(float4(posL, 1.0f), Skinning_world).xyzw;
    output.PosH = mul(float4(posL, 1.0f), Skinning_world).xyzw;
    output.PosH = mul(output.PosH, view_proj);

    output.NormalL = mul(input.NormalL, (float3x3) Skinning_world); // 이때는 역전치행렬을 쓴다
    output.TangentW = mul(input.TangentL, (float3x3) Skinning_world);
	//uv값
    output.UV = mul(float4(input.UV, 0.0f, 1.0f), Skinning_Tex);
    //output.PosH.w = 1.0f;
	return output;
}