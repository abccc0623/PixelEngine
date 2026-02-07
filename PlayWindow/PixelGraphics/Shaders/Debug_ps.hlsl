#define TEXTURE_MASK_MAIN   0x00000001 //메인텍스쳐 여부
#define TEXTURE_MASK_SUB    0x00000010 //서브텍스쳐 여부
#define TEXTURE_MASK_NOMAL  0x00000100 //노말텍스쳐 여부

struct PixelInputType
{
    float4 posH : SV_POSITION;
    float3 COLOR : COLOR;
};

float3 main(PixelInputType input) : SV_TARGET
{ 
    return input.COLOR;
}