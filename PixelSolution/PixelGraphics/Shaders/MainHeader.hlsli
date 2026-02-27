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
    float4x4    TexMatrix;
};

//스키닝 관련 버퍼
cbuffer SkinningBuffer : register(b2)
{
    float4x4 Skinning_world;
    float4x4 Skinning_Tex;
    float4x4 BoneOffset[96];
};

Texture2D MainTexture   : register(t0);
SamplerState SampleType : register(s0);