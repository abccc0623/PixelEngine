#pragma once
#include <cstdint>
#include "ResourceFactory.h"
struct RenderingData;
class RenderingFactory
{
public:
	RenderingFactory();
	~RenderingFactory();

	// ResourceFactory을(를) 통해 상속됨
	void Initialize();
	void Release();
	void Clear();
	void SetRenderingData(const RenderingData& mData);
protected:
	std::unordered_map<std::uint16_t, RenderingData*> renderingDataMap;
	void* GetResource(std::string name);

	
};

