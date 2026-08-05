
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

struct PixelInputType
{
    float4 posH : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 color : COLOR;
    //float3 NormalW	    : NORMAL;
    //float3 TangentW     : TANGENT;
};

float4 main(PixelInputType input) : SV_TARGET
{
  
    //법선이 단위벡터가 아닐 수 있으므로 다시 정규화 한다.
    //input.NormalW = normalize(input.NormalW);
   
    float4 textureColor = MainTexture.Sample(SampleType, input.UV);
    float4 finalColor = textureColor * input.color * Color;
    
    if (finalColor.a < 0.1f)
        discard;
    return finalColor;
}
