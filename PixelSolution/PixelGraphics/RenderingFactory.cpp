#include "pch.h"
#include "RenderingFactory.h"
#include "RenderringData.h"
RenderingFactory::RenderingFactory()
{

}

RenderingFactory::~RenderingFactory()
{

}

void RenderingFactory::Initialize()
{

}

void RenderingFactory::Release()
{

}

void* RenderingFactory::GetResource(std::string name)
{
	auto renderingData = new RenderingData();
	return renderingData;
}
