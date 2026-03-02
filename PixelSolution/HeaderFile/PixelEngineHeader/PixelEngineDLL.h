#pragma once
#include <windows.h>
typedef unsigned char byte;

#ifdef ENGINE_INTERFACE
#define PIXEL_ENGINEDLL __declspec(dllexport)
#else
#define PIXEL_ENGINEDLL __declspec(dllimport)
	#ifdef _DEBUG
		#pragma comment(lib ,"PixelEngine.lib")
	#else
		#pragma comment(lib ,"PixelEngine.lib")
	#endif
#endif

#ifdef PIXEL_EDITOR
#define EDITOR_ONLY_FUNC(name) void name() 
#define EDITOR_CALL(call) call
#else
#define EDITOR_ONLY_FUNC(name) inline void name() { } 
#define EDITOR_CALL(call) ((void)0)
#endif