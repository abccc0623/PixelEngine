#pragma once
#include "PixelEngineDLL.h"
#include <string>
#include <windows.h>
extern "C" PIXEL_ENGINEDLL bool EngineInitialize(HWND hWnd,int width,int height);
extern "C" PIXEL_ENGINEDLL void UpdateEngine();
extern "C" PIXEL_ENGINEDLL bool RunningEngineCheck();
extern "C" PIXEL_ENGINEDLL void ReleaseEngine();							
extern "C" PIXEL_ENGINEDLL void QuitWindow();
extern "C" PIXEL_ENGINEDLL bool ResizeEngine(int width,int height);

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
extern "C" PIXEL_ENGINEDLL bool LoadLuaScript(const std::string& path);
extern "C" PIXEL_ENGINEDLL bool LoadTexture(const std::string& path);
extern "C" PIXEL_ENGINEDLL bool CreateLuaAPIPath(const std::string& path);
