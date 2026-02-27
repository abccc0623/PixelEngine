#pragma once
#include <string>

class Transform;
class Vector3;
class GameObject;
class Camera;
class Renderer2D;
class LuaScript;
class BoxCollision2D;
class Collision2D;

#define BIND_TYPE_NAME(Type,Name) \
template <> struct TypeName<Type> { static std::string Get() { return Name; } }; 

template <typename T>
struct TypeName
{
	static std::string Get()
	{
		return typeid(T).name();
	}
};

BIND_TYPE_NAME(int, "number")
BIND_TYPE_NAME(double, "number")
BIND_TYPE_NAME(float, "number")
BIND_TYPE_NAME(std::string, "string")
BIND_TYPE_NAME(Transform, "Transform")
BIND_TYPE_NAME(Vector3, "Vector3")
BIND_TYPE_NAME(GameObject, "GameObject")
BIND_TYPE_NAME(Camera, "Camera")
BIND_TYPE_NAME(Renderer2D, "Renderer2D")
BIND_TYPE_NAME(LuaScript, "LuaScript")
BIND_TYPE_NAME(BoxCollision2D, "BoxCollision2D")
BIND_TYPE_NAME(Collision2D, "Collision2D")