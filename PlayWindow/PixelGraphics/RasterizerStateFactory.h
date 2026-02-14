#pragma once
#include "ResourceFactory.h"
struct RasterizerStateResources;
class RasterizerStateFactory : public ResourceFactory
{
public:
	RasterizerStateFactory();
	~RasterizerStateFactory();

	void Initialize() override;
	void Release() override;
private:
	RasterizerStateResources* CreateRasterizerState_Solid();
	RasterizerStateResources* CreateRasterizerState_Wire();
	std::unordered_map<std::string, RasterizerStateResources*> rasterizerMap;

	void* GetResource(std::string name) override;
};

