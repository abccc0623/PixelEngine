
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

    //¿ùµåº¯È¯
    output.posH = mul(float4(input.position.xyz, 1.0f), world);
    output.posH = mul(float4(output.posH.xyz, 1.0f), world);
    output.posH = mul(float4(output.posH.xyz, 1.0f), view_proj);
    //uv°ª
    output.UV = mul(float4(input.UV.xy, 0.0f, 1.0f), TexMatrix);
    //³ë¸»
    output.NormalW = mul(input.NormalL, (float3x3) view_proj);
    //ÅºÁ¨Æ®
    output.TangentW = mul(input.TangentL, (float3x3) world);
 
    return output;
}