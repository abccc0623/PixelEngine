#include "Renderer.h"
#include "GameObject.h"
#include "Transform.h"
#include "Matrix.h"
#include "PixelGraphicsAPI.h"
#include "PixelEngineAPI.h"
#include <iostream>
Renderer::Renderer() 
{
	
}

Renderer::~Renderer()
{

}

void Renderer::Awake()
{
    float BoxSize = 0.1f;
    Vertex_Debug mVertex[8];
    int IndexList[24];
    mVertex[0].Position[0] = -BoxSize;
    mVertex[0].Position[1] = BoxSize;
    mVertex[0].Position[2] = BoxSize;
    mVertex[1].Position[0] = -BoxSize;
    mVertex[1].Position[1] = BoxSize;
    mVertex[1].Position[2] = -BoxSize;
    mVertex[2].Position[0] = BoxSize;
    mVertex[2].Position[1] = BoxSize;
    mVertex[2].Position[2] = BoxSize;
    mVertex[3].Position[0] = BoxSize;
    mVertex[3].Position[1] = BoxSize;
    mVertex[3].Position[2] = -BoxSize;
    mVertex[4].Position[0] = -BoxSize;
    mVertex[4].Position[1] = -BoxSize;
    mVertex[4].Position[2] = BoxSize;
    mVertex[5].Position[0] = -BoxSize;
    mVertex[5].Position[1] = -BoxSize;
    mVertex[5].Position[2] = -BoxSize;
    mVertex[6].Position[0] = BoxSize;
    mVertex[6].Position[1] = -BoxSize;
    mVertex[6].Position[2] = BoxSize;
    mVertex[7].Position[0] = BoxSize;
    mVertex[7].Position[1] = -BoxSize;
    mVertex[7].Position[2] = -BoxSize;

    for (size_t i = 0; i < 8; i++)
    {
        mVertex[i].Color[0] = 1.0f;
        mVertex[i].Color[1] = 0.0f;
        mVertex[i].Color[2] = 0.0f;
    }


    IndexList[0] = 0; IndexList[1] = 1; IndexList[2] = 0;
    IndexList[3] = 2; IndexList[4] = 0; IndexList[5] = 4;
    IndexList[6] = 1; IndexList[7] = 3; IndexList[8] = 1;
    IndexList[9] = 5; IndexList[10] = 3; IndexList[11] = 7;
    IndexList[12] = 3; IndexList[13] = 2; IndexList[14] = 2;
    IndexList[15] = 6; IndexList[16] = 4; IndexList[17] = 5;
    IndexList[18] = 4; IndexList[19] = 6; IndexList[20] = 6;
    IndexList[21] = 7; IndexList[22] = 7; IndexList[23] = 5;
    index = CreateDebugModel(mVertex, 8, IndexList, 24);
    rendering = GetRenderingData();
    rendering->Type = DEBUG;
    //rendering->ModelIndex = index;
}

void Renderer::Start()
{
    transform = targetObject->GetModule<Transform>();
}

void Renderer::Update()
{

}

void Renderer::LastUpdate()
{
    for (size_t i = 0; i < 16; i++)
    {
        rendering->World[i] = transform->World_Matrix._m[i];
    }
}
