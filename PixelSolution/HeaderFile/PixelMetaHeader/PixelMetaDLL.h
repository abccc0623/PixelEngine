#pragma once
#ifdef PIXELMETA_EXPORTS
#define PIXEL_META_DLL __declspec(dllexport)
#else
#define PIXEL_META_DLL __declspec(dllimport)
#ifdef _DEBUG
#pragma comment(lib ,"PixelMeta")
#else
#pragma comment(lib ,"PixelMeta")
#endif
#endif 