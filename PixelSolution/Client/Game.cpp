#include "Game.h"
#include <string>
#include <iostream>
#include "PixelEngineAPI.h"
#include "Core/Module.h"
#include "Core/GameObject.h"

#include "Module/Renderer2D.h"
#include "Module/Transform.h"

Transform* transform = nullptr;

void Image(std::string name, float x, float y);
void Start()
{
	LoadTexture("./Asset/tree.png");
	LoadTexture("./Asset/back.png");

	GameObject* camera = CreateGameObject("Camera");
	AddModule(camera, GetMetaClass("Camera"));

	Image("tree",0,0);
	Image("back",1,0);
}

void Update()
{
	
}

void Image(std::string name,float x, float y)
{
	GameObject* Object = CreateGameObject("Ground");
	Module* targetR = AddModule(Object, GetMetaClass("Renderer2D"));
	Module* targetT = GetModule(Object, GetMetaClass("Transform"));
	Renderer2D* render = reinterpret_cast<Renderer2D*>(targetR);
	Transform* transform = reinterpret_cast<Transform*>(targetT);
	render->SetTexture(name);
	transform->Position.X = x;
	transform->Position.Y = y;
}
