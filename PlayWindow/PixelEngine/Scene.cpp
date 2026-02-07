#include "Scene.h"
#include "PixelGraphicsAPI.h"
#include "PixelEngineAPI.h"
#include "GameObject.h"
#include "PixelEngine.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Transform.h"
#include "DebugCamera.h"
#include <iostream>
extern PixelEngine* Engine;
Scene::Scene():
    manager(nullptr),
    quad(nullptr),
    cam(nullptr)
{

}

Scene::~Scene()
{

}
void Scene::Initialize()
{
    GameObject* q = Engine->CreateGameObject();
    q->AddModule<Renderer2D>();

    GameObject* obj = Engine->CreateGameObject();
    obj->AddModule<DebugCamera>();
}

void Scene::Start()
{
    
}

void Scene::Update()
{
   
}

void Scene::Release()
{

}
