#pragma once
#include "ResourceFactory.h"
class BufferFactory
{
public:
	BufferFactory();
	~BufferFactory();

	void Initialize();
	void Release();
	void Clear();

	void* GetResource(std::string name);
};

