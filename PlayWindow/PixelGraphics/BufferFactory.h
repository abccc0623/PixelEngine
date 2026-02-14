#pragma once
#include "ResourceFactory.h"
#include "ShaderLoader.h"
class BufferFactory : public ResourceFactory
{
public:
	BufferFactory();
	~BufferFactory();

	void Initialize() override;
	void Release() override;

	void* GetResource(std::string name) override;
};

