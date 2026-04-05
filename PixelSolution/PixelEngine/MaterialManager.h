#pragma once
#include <string>
#include <unordered_map>
#include "EngineManager.h"
using ObjectID = size_t;
class MaterialManager : public EngineManager
{
public:
	MaterialManager();
	virtual ~MaterialManager();

	// EngineManager을(를) 통해 상속됨
	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	ObjectID Load(const std::string& path);
	ObjectID Get(const std::string& name);

private:
	std::unordered_map<std::string, ObjectID> MaterialMap;
};

