#pragma once
#include <windows.h>
typedef unsigned char byte;

#ifdef ENGINE_INTERFACE
#define PIXEL_ENGINEDLL __declspec(dllexport)
#else
#define PIXEL_ENGINEDLL __declspec(dllimport)
#endif