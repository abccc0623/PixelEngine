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

    // 2. 월드 좌표를 화면(NDC) 좌표로 변환 (View & Projection)
    output.posH = mul(worldPos, view_proj);
    output.Tex = input.Color;
    return output;
}