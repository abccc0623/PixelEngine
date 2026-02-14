#pragma once
#include "ResourceFactory.h"
struct RenderingData;
class RenderingFactory : public ResourceFactory
{
public:
	RenderingFactory();
	~RenderingFactory();

	// ResourceFactory을(를) 통해 상속됨
	void Initialize() override;
	void Release() override;
protected:
	std::unordered_map<Handle16, RenderingData*> renderingDataMap;
	void* GetResource(std::string name) override;
};

