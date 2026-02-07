#include "MainHeader.hlsli"

struct PixelInputType
{
    float4 PosH         : SV_POSITION;
    float3 PosL         : POSITION;
    float2 UV           : TEXCOORD;
    float3 NormalL      : NORMAL;
    float3 TangentW     : TEXCOORD2;
};

float4 main(PixelInputType input) : SV_Target
{
   // Tex = MainTexture.Sample(SampleType, input.UV);
    float4 textureColor = { 1.0f, 1.0f, 1.0f,1.0f};
    //output.color    = Tex;
    //output.tangent  = float4(input.TangentW.xyz, 1);
    //output.Pos      = float4(input.PosL.xyz, 1);
    //output.normal   = float4(input.NormalL, 1.0f);
    return textureColor;
}