#include "Camera.h"
#include "Transform.h"
#include "GameObject.h"
#include "PixelGraphicsAPI.h"
#include "LuaManager.h"
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
		rendering->cameraOption.Projection = CameraOption::ProjectionType::Orthographic;
	}
}

void Camera::PerspectiveProjection()
{
	if (rendering != nullptr)
	{
		rendering->cameraOption.Projection = CameraOption::ProjectionType::Perspective;
	}
}

void Camera::RegisterLua()
{
	auto state = GetLuaState();
	state->new_usertype<Camera>("Camera", sol::base_classes, sol::bases<Module, BaseModule>(),
		"OrthographicProjection", [](Camera& obj) {obj.OrthographicProjection();},
		"PerspectiveProjection", [](Camera& obj) {obj.PerspectiveProjection();}
	);
	std::vector<std::string> functionName = std::vector<std::string>();
	functionName.push_back("function Camera.OrthographicProjection(...)");
	functionName.push_back("function Camera.PerspectiveProjection(...)");
	AddLuaAPI("Camera", functionName);
}
