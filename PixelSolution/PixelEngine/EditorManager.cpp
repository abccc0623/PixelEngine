#include "pch.h"
#include "EditorManager.h"
#include "PixelEngineAPI.h"
#include "GenerateManager.h"
#include "LuaManager.h"
#include "PixelEngine.h"

extern PixelEngine* Engine;
void EditorManager::Initialize()
{

}

void EditorManager::Update()
{

}

void EditorManager::Release()
{

}

void EditorManager::Clear()
{

}

void EditorManager::Notify(std::int32_t eventType, std::string content)
{
	switch (eventType)
	{
	case (std::int32_t)EditorEventType::GenerateEngineFile:
		GenerateManager* Generate = Engine->GetFactory<GenerateManager>();
		Generate->JsonGenerate(content.c_str());
		Engine->GetFactory<LuaManager>()->ReadEngineGenerateFile();
		break;
	}
}
