#pragma once
#include "PixelEngineDLL.h"
class GameObject;
class PObject;


typedef void* PixelWindowHandle;

extern "C" PIXEL_ENGINEDLL bool EngineInitialize(PixelWindowHandle hWnd,int width,int height);
extern "C" PIXEL_ENGINEDLL void UpdateEngine();
extern "C" PIXEL_ENGINEDLL void ReleaseEngine();							
extern "C" PIXEL_ENGINEDLL void QuitWindow();
extern "C" PIXEL_ENGINEDLL bool ResizeEngine(int width,int height);

//오브젝트 컨트롤
extern "C" PIXEL_ENGINEDLL PObject* CreateObject(const char* name = "GameObject");
extern "C" PIXEL_ENGINEDLL int GetObjectFieldMaxCount(PObject* Obj);
extern "C" PIXEL_ENGINEDLL int GetObjectMethodMaxCount(PObject* Obj);
extern "C" PIXEL_ENGINEDLL const char* GetObjectTypeName(PObject* Obj);
extern "C" PIXEL_ENGINEDLL const char* GetObjectParentName(PObject* Obj);
extern "C" PIXEL_ENGINEDLL const char* GetObjectChildName(PObject* Obj);
extern "C" PIXEL_ENGINEDLL const char* GetObjectFieldName(PObject* Obj, int index);
extern "C" PIXEL_ENGINEDLL const char* GetObjectFieldType(PObject* Obj, int index);



extern "C" PIXEL_ENGINEDLL GameObject* CreateGameObject(const char* name = "GameObject");
/**/

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

//Lua
extern "C" PIXEL_ENGINEDLL bool LoadLuaFile(const char* path);
extern "C" PIXEL_ENGINEDLL bool LoadTexture(const char* path);
extern "C" PIXEL_ENGINEDLL bool CreateLuaAPIPath(const char* path);



typedef void(__stdcall* LogCallbackFunc)(const char* message, int level);
typedef void(__stdcall* SceneChangeCallbackFunc)();
//Scene
extern "C" PIXEL_ENGINEDLL bool CreateScene(const char* sceneName);
extern "C" PIXEL_ENGINEDLL bool ChangeScene(const char* sceneName);
extern "C" PIXEL_ENGINEDLL void RegisterSceneObjectChange(SceneChangeCallbackFunc callBack);
extern "C" PIXEL_ENGINEDLL GameObject** GetAllSceneObjects(int* outCount);

extern "C" PIXEL_ENGINEDLL void SaveScene();
extern "C" PIXEL_ENGINEDLL void LoadScene(const char* sceneName);


extern "C" PIXEL_ENGINEDLL void RegisterLogCallback(LogCallbackFunc callback);
void DispatchNativeLog(const char* msg, int level);