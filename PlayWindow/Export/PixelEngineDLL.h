#pragma once

#ifdef ENGINE_INTERFACE
#define PIXEL_ENGINEDLL __declspec(dllexport)
#else
#define PIXEL_ENGINEDLL __declspec(dllimport)
#endif