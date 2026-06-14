#pragma once
#include "EngineManager.h"

class EditorManager : public EngineManager
{
public:
	void Initialize() override;
	void Update() override;
	void Release() override;
	void Clear() override;

	void Notify(std::int32_t eventType, std::string content);
private:

};

