#include "BindingClass.h"
#include "GraphicsEngine.h"
BindingClass::BindingClass()
{
	engine = nullptr;
}

BindingClass::~BindingClass()
{

}

void BindingClass::SetEngine(GraphicsEngine* graphicsEngine)
{
	engine = graphicsEngine;
}

bool BindingClass::ModelCheck(Handle64 now, Handle64 prev)
{
	Handle64 data1 = now;
	Handle64 data2 = prev;
	if ((data1 & 0xFFFF) == (data2 & 0xFFFF))
	{
		return true;
	}
	return false;
}

bool BindingClass::ShaderCheck(Handle64 now, Handle64 prev)
{
	Handle64 data1 = now;
	Handle64 data2 = prev;
	if (((data1 >> 16) & 0x1FFFF) == ((data2 >> 16) & 0x1FFFF))
	{
		return true;
	}
	return false;
}

bool BindingClass::TextureCheck(Handle64 now, Handle64 prev)
{
	Handle64 data1 = now;
	Handle64 data2 = prev;
	if (((data1 >> 32) & 0xFFFF) == ((data2 >> 32) & 0xFFFF))
	{
		return true;
	}
	return false;
}
