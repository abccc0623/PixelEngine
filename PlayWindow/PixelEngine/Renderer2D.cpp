#include "Renderer2D.h"
#include "GameObject.h"
#include "Transform.h"
#include "PixelEngineAPI.h"
#include "PixelGraphicsAPI.h"
Renderer2D::Renderer2D()
{

}

Renderer2D::~Renderer2D()
{

}

void Renderer2D::Awake()
{

}

void Renderer2D::Start()
{
	transform = targetObject->GetModule<Transform>();
	rendering = GetRenderingData();
	rendering->Type = QUAD;
	rendering->ModelIndex = 10;
}

void Renderer2D::Update()
{
	transform->AddRotation(Vector3(0, 0, GetDeltaTime() * 120));
}

void Renderer2D::LastUpdate()
{
	Matrix m = transform->GetWorldMatrix();
	for (int i = 0; i < 16; i++)
	{
		rendering->World[i] = m._m[i];
	}
}
