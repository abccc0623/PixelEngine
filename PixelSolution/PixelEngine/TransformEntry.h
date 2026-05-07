#pragma once
#include "PixelMath.h"
//16의 배수 48byte
struct alignas(16) TransformEntry
{
    Pixel::Vector4 Position;    
    Pixel::Quaternion Rotation; 
    Pixel::Vector4 Scale;
};

//그래픽 엔진에서 사용될 구조체
struct TransformMatrixEntry
{
    Pixel::Matrix4x4 World;
};

