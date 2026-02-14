#include "BufferFactory.h"
#include "PixelResources.h"
BufferFactory::BufferFactory()
{

}

BufferFactory::~BufferFactory()
{

}

void BufferFactory::Initialize()
{

}

void BufferFactory::Release()
{

}

void* BufferFactory::GetResource(std::string name)
{
	auto target = ShaderLoader::mContextBufferMap.find(name);
	if (target != ShaderLoader::mContextBufferMap.end())
	{
		return target->second;
	}

	return nullptr;
}
