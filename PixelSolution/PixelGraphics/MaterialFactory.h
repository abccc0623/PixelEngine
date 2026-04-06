#pragma once
#include "ResourceFactory.h"
struct MaterialResources;
class MaterialFactory : public ResourceFactory
{
public:
	void Initialize() override;
	void Release() override;
	void Clear() override;
	void* GetResource(std::string name) override;
	void* GetResource(Handle16 key) override;

	Handle16 SetResource(std::string name) override;
private:
	std::unordered_map<std::string, MaterialResources*> materialMap;
};

