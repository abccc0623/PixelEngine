#pragma once
#define DLLEXPORT extern "C" __declspec(dllexport)
#include "Platform.h"

// 1. 핸들 및 기본 타입 정의
struct HWND__;
typedef HWND__* HWND;
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;


// 2. 이게 빠지면 WndProc에서 에러가 납니다!
#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

// 3. 상속 시 public을 붙여야 C++ 다형성을 제대로 쓸 수 있습니다.
class WindowPlatform : public Platform
{
public:
    WindowPlatform() = default;
    virtual ~WindowPlatform() = default;
private:
    HINSTANCE g_hInstance = nullptr;
public:
    HWND g_hWnd = nullptr;
public:
    void Initialize() override;
    void Update() override;
    void Release() override;
};