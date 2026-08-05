#pragma once

#include <string>
#include <unordered_map>

#include "EngineManager.h"

using ObjectID = size_t;

class FontManager : public EngineManager
{
public:
	FontManager();
	virtual ~FontManager();

	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	void Load(const std::string& path);
	ObjectID Get(const std::string& name);

private:
	std::unordered_map<std::string, ObjectID> FontMap;
};
