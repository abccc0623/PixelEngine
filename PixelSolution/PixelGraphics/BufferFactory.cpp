#include "pch.h"
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
	for (auto K : ShaderLoader::mContextBufferMap)
	{
		K.second->buffer->Release();
		delete K.second;
	}
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
