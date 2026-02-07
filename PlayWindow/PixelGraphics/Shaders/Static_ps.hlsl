
struct PixelInputType
{
    float4 posH		    : SV_POSITION;
    float2 UV           : TEXCOORD;
    float3 NormalW	    : NORMAL;
    float3 TangentW     : TANGENT;
};

float4 main(PixelInputType input) : SV_TARGET
{
  
    //법선이 단위벡터가 아닐 수 있으므로 다시 정규화 한다.
    input.NormalW = normalize(input.NormalW);
   
    float4 textureColor = { 1.0f, 1.0f, 1.0f, 1.0f};
    float4 TexColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    float4 Tex01 = { 1.0f, 1.0f, 1.0f, 1.0f };
    float4 Tex02 = { 1.0f, 1.0f, 1.0f, 1.0f };

    return textureColor;
}

