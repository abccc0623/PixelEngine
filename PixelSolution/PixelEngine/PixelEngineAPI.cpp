#include "pch.h"
#include "PixelEngineAPI.h" 
#include "PixelGraphicsAPI.h"
#include "PixelEngine.h" 
#include <cctype>
#include <filesystem>
#include <windows.h>
#include "KeyInputManager.h"
#include "LuaManager.h"
#include "GenerateManager.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ResourceManager.h"
#include "TimeManager.h"
#include "JsonManager.h"
#include "EventManager.h"
#include "PixelMetaAPI.h"
#include "EditorManager.h"
#include "Log.h"


PixelEngine* Engine = nullptr;
LogCallbackFunc g_logCallback = nullptr;
SceneChangeCallbackFunc g_SceneObjectChangeCallBack = nullptr;

ECS::Registry* GetRegistry()
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	return sceneManager->GetRegistry();
}

bool EngineInitialize(PixelWindowHandle hWnd, int width, int height, const char* assetPath)
{
	Engine = new PixelEngine();
	Engine->Initialize((HWND)hWnd, width, height, std::string(assetPath));
	return true;
}

void UpdateEngine()
{
	if (Engine != nullptr)
	{
		Engine->Update();
	}
}

bool RunningEngineCheck()
{
	if (Engine != nullptr)
	{
		return Engine->RunningCheck();
	}
	return false;
}

void ReleaseEngine()
{
	if (Engine != nullptr)
	{
		Engine->ReleaseShared();
	}
	ReleaseMetaType();
	delete Engine;
}

void QuitWindow()
{

}

bool ResizeEngine(int width, int height)
{
	if (Engine != nullptr)
	{
		Engine->Resize(width, height);
	}
	return true;
}

void SetWindowFocus(bool focus)
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		key->Focus = focus;
	}
}

void Reload()
{
	if (Engine != nullptr)
	{
		PixelLog::Info("[Global] Reload Script");
		Engine->Clear();
	}
}

void EditorNotify(std::int32_t eventType, const char* content)
{
	if (Engine != nullptr)
	{
		auto editor = Engine->GetFactory<EditorManager>();
		editor->Notify(eventType, std::string(content));
	}
}

bool GetKeyDown(char number)
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->GetKeyDown(number);
	}
	return false;
}

bool GetKeyUp(char number)
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->GetKeyUp(number);
	}
	return false;
}

bool GetKey(char number)
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->GetKey(number);
	}
	return false;
}

int GetMousePosition_X()
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->MousePosition_X();
	}
	return -1;
}

int GetMousePosition_Y()
{
	if (Engine != nullptr)
	{
		auto key = Engine->GetFactory<KeyInputManager>();
		return key->MousePosition_Y();
	}
	return -1;
}

float GetDeltaTime()
{
	if (Engine != nullptr)
	{
		auto time = Engine->GetFactory<TimeManager>();
		return time->GetDeltaTime();
	}
	return -1;
}

double GetTotalTime()
{
	if (Engine != nullptr)
	{
		auto time = Engine->GetFactory<TimeManager>();
		return time->GetTotalTime();
	}
	return -1;
}

int GetFPS()
{
	if (Engine != nullptr)
	{
		auto time = Engine->GetFactory<TimeManager>();
		return time->GetFPS();
	}
	return -1;
}

void LogInfo(const char* log)
{
	PixelLog::Info(log);
}

void LogError(const char* log)
{
	PixelLog::Error(log);
}

void LogWarning(const char* log)
{
	PixelLog::Warn(log);
}

bool LoadTexture(const char* path)
{
	std::string strPath(path);
	if (Engine != nullptr)
	{
		auto resource = Engine->GetFactory<ResourceManager>();
		resource->Load(TEXTURE, strPath);
		return true;
	}
	return false;
}

static void ImportFile(const std::filesystem::path& path)
{
	std::string targetPath = path.generic_string();
	std::string fileName = path.stem().string();
	std::string ext = path.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char character)
		{
			return static_cast<char>(std::tolower(character));
		});

	if (ext == ".lua" || ext == ".scene" || ext == ".pxm")
	{
		auto lua = Engine->GetFactory<LuaManager>();
		lua->ImportLua(targetPath, fileName, ext);
	}
	else if (ext == ".png" || ext == ".jpg")
	{
		auto resource = Engine->GetFactory<ResourceManager>();
		resource->Load(TEXTURE, targetPath);
	}
	else if (ext == ".mat")
	{
		auto resource = Engine->GetFactory<ResourceManager>();
		resource->Load(MATERIAL, targetPath);
	}
	else if (ext == ".json")
	{
		auto jsonFile = Engine->GetFactory<JsonManager>();
		jsonFile->Load(targetPath);
	}
}

void Import(const char* path)
{
	if (Engine == nullptr || path == nullptr || path[0] == '\0')
	{
		return;
	}

	std::filesystem::path targetPath(path);
	std::error_code error;
	std::filesystem::file_status status = std::filesystem::status(targetPath, error);
	if (error || !std::filesystem::exists(status))
	{
		PixelLog::Error("Path does not exist: " + targetPath.generic_string());
		return;
	}

	if (std::filesystem::is_regular_file(status))
	{
		ImportFile(targetPath);
		return;
	}

	if (!std::filesystem::is_directory(status))
	{
		return;
	}

	std::filesystem::recursive_directory_iterator iterator(
		targetPath,
		std::filesystem::directory_options::skip_permission_denied,
		error);
	std::filesystem::recursive_directory_iterator end;
	while (iterator != end)
	{
		if (error)
		{
			PixelLog::Warn("Failed to scan asset directory: " + error.message());
			error.clear();
			iterator.increment(error);
			continue;
		}

		if (iterator->is_regular_file(error) && !error)
		{
			ImportFile(iterator->path());
		}
		else
		{
			error.clear();
		}

		iterator.increment(error);
	}
}


ECS::Entity* FindEntity(unsigned int id)
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	return sceneManager->FindEntity(id);
}

void ActiveEntity(unsigned int id, bool active)
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	sceneManager->ActiveEntity(id, active);
}

unsigned int CreateEntity(const char* script)
{
	std::string scriptName(script);
	auto sceneManager = Engine->GetFactory<SceneManager>();
	auto luaManager = Engine->GetFactory<LuaManager>();
	auto id = sceneManager->CreateEntity(scriptName);
	return id;
}

void DestroyEntity(unsigned int id)
{
	auto sceneManager = Engine->GetFactory<SceneManager>();
	uint32_t u(id);
	sceneManager->DestroyEntity(u);
}

void BindLuaEvent(unsigned int id, const char* eventKey, const char* functionName)
{
	auto eventManager = Engine->GetFactory<EventManager>();
	if (eventManager != nullptr)
	{
		std::string key(eventKey);
		std::string func(functionName);
		eventManager->BindLuaEvent(id, key, func);
	}
}

//void CallLuaEvent(const char* eventName, sol::table luaTable)
//{
//	auto eventManager = Engine->GetFactory<EventManager>();
//}


void RegisterMessage(GameObject* target, EventType type)
{
	auto eventManager = Engine->GetFactory<EventManager>();
	//eventManager->RegisterMessage(target, type);
}

void RegisterCustomMessage(GameObject* target, const char* type)
{
	std::string eventType(type);
	auto eventManager = Engine->GetFactory<EventManager>();
	//eventManager->RegisterMessageCustom(target, eventType);
}

void UnregisterMessage(GameObject* target, EventType type)
{
	auto eventManager = Engine->GetFactory<EventManager>();
	//eventManager->UnregisterMessage(target, type);
}

void UnregisterCustomMessage(GameObject* target, const char* type)
{
	std::string eventType(type);
	auto eventManager = Engine->GetFactory<EventManager>();
	//eventManager->UnregisterMessageCustom(target, eventType);
}

void BackgroundColor(int R, int G, int B)
{
	PixelGraphicsBackgroundColor(R, G, B);
}


Module* GetModuleByString(GameObject* target, const char* name)
{
	return nullptr;
}

#pragma region MetaType
//PClass* GetMetaClass(const char* className)
//{
//	std::string targetName(className);
//	return GetClass(targetName);
//}
//const char* GetMemberName(PClass* targetClass, int index)
//{
//	return GetClassMemberName(targetClass, index).c_str();
//}
//const char* GetMemberType(PClass* targetClass, int index)
//{
//	return GetClassMemberType(targetClass, index).c_str();
//}
//const char* GetMethodName(PClass* targetClass, int index)
//{
//	return GetClassMethodName(targetClass,index).c_str();
//}
//const char* GetMethodReturnType(PClass* targetClass, int index)
//{
//	return GetClassMethodReturnType(targetClass, index).c_str();
//}
//const char* GetMethodGetPropertyType(PClass* targetClass, int index, int propertyIndex)
//{
//	return GetClassMethodGetPropertyType(targetClass, index, propertyIndex).c_str();
//}
//int GetMethodPropertyCount(PClass* targetClass, int index)
//{
//	return GetClassMethodPropertyCount(targetClass, index);
//}
void LuaGenerate(const char* outPath)
{
	if (Engine != nullptr)
	{
		GenerateManager* Generate = Engine->GetFactory<GenerateManager>();
		Generate->LuaGenerate(outPath);
	}
}
void LSPGenerate(const char* outPath)
{
	if (Engine != nullptr)
	{
		GenerateManager* Generate = Engine->GetFactory<GenerateManager>();
		Generate->LSPGenerate(outPath);
	}
}
void JsonGenerate(const char* outPath)
{
	if (Engine != nullptr)
	{
		GenerateManager* Generate = Engine->GetFactory<GenerateManager>();
		Generate->JsonGenerate(outPath);
	}
}
//
//int GetMemberCount(PClass* targetClass)
//{
//	return GetClassMemberCount(targetClass);
//}
//int GetMethodCount(PClass* targetClass)
//{
//	return GetClassMethodCount(targetClass);
//}
#pragma endregion

bool CreateScene(const char* sceneName)
{
	std::string strPath(sceneName);
	if (Engine != nullptr)
	{
		SceneManager* scene = Engine->GetFactory<SceneManager>();
		scene->CreateScene(strPath);
	}
	return true;
}

void LoadScene(const char* sceneName)
{
	if (Engine != nullptr)
	{
		SceneManager* scene = Engine->GetFactory<SceneManager>();
		auto path = std::string(sceneName);
	}
}

void RegisterLogCallback(LogCallbackFunc callback)
{
	g_logCallback = callback;
}

void DispatchNativeLog(const char* msg, int level)
{
	if (g_logCallback)
	{
		g_logCallback(msg, level);
	}
}

void ChangeScene(const char* sceneName)
{
	std::string strPath(sceneName);
	if (Engine != nullptr)
	{
		SceneManager* scene = Engine->GetFactory<SceneManager>();
		scene->ChangeScene(strPath);
	}
}

void  RegisterSceneObjectChange(SceneChangeCallbackFunc callBack)
{
	g_SceneObjectChangeCallBack = callBack;
}

