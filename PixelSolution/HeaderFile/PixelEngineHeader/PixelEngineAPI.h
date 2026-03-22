#pragma once
#include "PixelEngineDLL.h"
class GameObject;
class PObject;
class PClass;
class Module;
typedef void* PixelWindowHandle;

extern "C" PIXEL_ENGINEDLL bool EngineInitialize(PixelWindowHandle hWnd,int width,int height);
extern "C" PIXEL_ENGINEDLL void UpdateEngine();
extern "C" PIXEL_ENGINEDLL void ReleaseEngine();							
extern "C" PIXEL_ENGINEDLL void QuitWindow();
extern "C" PIXEL_ENGINEDLL bool ResizeEngine(int width,int height);
extern "C" PIXEL_ENGINEDLL void SetWindowFocus(bool focus);
extern "C" PIXEL_ENGINEDLL void Reload();



extern "C" PIXEL_ENGINEDLL void Import(const char* path);
extern "C" PIXEL_ENGINEDLL GameObject* CreateGameObject(const char* name);




#pragma region MetaType
extern "C" PIXEL_ENGINEDLL PClass* GetMetaClass(const char* className);
extern "C" PIXEL_ENGINEDLL int GetMemberCount(PClass* targetClass);
extern "C" PIXEL_ENGINEDLL int GetMethodCount(PClass* targetClass);
extern "C" PIXEL_ENGINEDLL const char* GetMemberName(PClass* targetClass, int index);
extern "C" PIXEL_ENGINEDLL const char* GetMemberType(PClass* targetClass, int index);
extern "C" PIXEL_ENGINEDLL const char* GetMethodName(PClass* targetClass, int index);
extern "C" PIXEL_ENGINEDLL const char* GetMethodReturnType(PClass* targetClass, int index);
extern "C" PIXEL_ENGINEDLL const char* GetMethodGetPropertyType(PClass* targetClass, int index,int propertyIndex);
extern "C" PIXEL_ENGINEDLL int GetMethodPropertyCount(PClass* targetClass, int index);
#pragma endregion

extern "C" PIXEL_ENGINEDLL void GenerateLuaBindCode(const char* outPath);
extern "C" PIXEL_ENGINEDLL void GenerateLuaAPICodeJson(const char* outPath);

//KeyInput
extern "C" PIXEL_ENGINEDLL bool GetKeyDown(char number);		
extern "C" PIXEL_ENGINEDLL bool GetKeyUp(char number);
extern "C" PIXEL_ENGINEDLL bool GetKey(char number);
extern "C" PIXEL_ENGINEDLL int GetMousePosition_X();
extern "C" PIXEL_ENGINEDLL int GetMousePosition_Y();

//Time
extern "C" PIXEL_ENGINEDLL float GetDeltaTime();
extern "C" PIXEL_ENGINEDLL double GetTotalTime();
extern "C" PIXEL_ENGINEDLL int GetFPS();


typedef void(__stdcall* LogCallbackFunc)(const char* message, int level);
typedef void(__stdcall* SceneChangeCallbackFunc)();
//Scene
extern "C" PIXEL_ENGINEDLL bool CreateScene(const char* sceneName);
extern "C" PIXEL_ENGINEDLL void ChangeScene(const char* sceneName);
extern "C" PIXEL_ENGINEDLL void RegisterSceneObjectChange(SceneChangeCallbackFunc callBack);
extern "C" PIXEL_ENGINEDLL GameObject** GetAllSceneObjects(int* outCount);

extern "C" PIXEL_ENGINEDLL void SaveScene();
extern "C" PIXEL_ENGINEDLL void LoadScene(const char* sceneName);


extern "C" PIXEL_ENGINEDLL void RegisterLogCallback(LogCallbackFunc callback);
void DispatchNativeLog(const char* msg, int level);