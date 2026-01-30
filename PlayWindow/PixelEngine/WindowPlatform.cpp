#include "WindowPlatform.h"
#include "PixelEngineAPI.h"
#include <windows.h>
#include <process.h>
#include <iostream>

LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

unsigned int __stdcall WindowThread(void* pArguments)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"PixelEngineClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    WindowPlatform* pThis = (WindowPlatform*)pArguments;

    RegisterClass(&wc);

    pThis->g_hWnd = CreateWindowEx(0, L"PixelEngineClass", L"Pixel Engine Render Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, wc.hInstance, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

void WindowPlatform::Initialize()
{
    _beginthreadex(NULL, 0, WindowThread,this, 0, NULL);
}

void WindowPlatform::Update()
{
    //std::cout<< "Hello, World!" << std::endl;
}

void WindowPlatform::Release()
{
    
}
