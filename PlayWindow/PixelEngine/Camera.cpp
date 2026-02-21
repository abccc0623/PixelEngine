#include "Camera.h"
#include "Transform.h"
#include "GameObject.h"
#include "PixelGraphicsAPI.h"
#include "LuaManager.h"
#include "BindManager.h"
Camera::Camera():
	rendering(nullptr)
{
	rendering = GetRenderingData();
	rendering->Type = CAMERA;
}
Camera::~Camera()
{
	DeleteRenderingData(rendering);
}

void Camera::Start()
{
	transform = targetObject->GetModule<Transform>();
	transform->AddPosition(transform->GetLookVector()*-3);
}

void Camera::LastUpdate()
{
	Matrix local = transform->GetLocal();
	for (int i = 0; i < 16; i++)
	{
		rendering->World[i] = local._m[i];
	}
}

void Camera::OrthographicProjection()
{
	if (rendering != nullptr)
	{
		rendering->camera.Projection = ProjectionType::Orthographic;
	}
}

void Camera::PerspectiveProjection()
{
	if (rendering != nullptr)
	{
		rendering->camera.Projection = ProjectionType::Perspective;
	}
}

std::string Camera::RegisterLua()
{
	auto state = GetLuaState();
	state->new_usertype<Camera>("Camera", sol::base_classes, sol::bases<Module, PixelObject>(),
		"OrthographicProjection", [](Camera& obj) {obj.OrthographicProjection();},
		"PerspectiveProjection", [](Camera& obj) {obj.PerspectiveProjection();}
	);

	std::string main = "";
	main += BindManager::ExportLuaAPIHeader<Camera>();
	main += BindManager::ExportLuaAPIFromFunc("OrthographicProjection",&Camera::OrthographicProjection, "");
	main += BindManager::ExportLuaAPIFromFunc("PerspectiveProjection",&Camera::PerspectiveProjection, "");
	return main;
}
