#pragma once
#define DLLEXPORT extern "C" __declspec(dllexport)
#include "Platform.h"
#include <string>

struct HWND__;
typedef HWND__* HWND;
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

class RenderingData;
class WindowPlatform : public Platform
{
public:
    WindowPlatform() = default;
    virtual ~WindowPlatform() = default;
private:
    HINSTANCE g_hInstance = nullptr;
public:
    static int width;
    static int height;
    HWND g_hWnd = nullptr;
    std::wstring windowTitle = L"HyungSun Window";
public:
    void Initialize() override;
    void Update() override;
    void Release() override;
    void QuitWindow();
};