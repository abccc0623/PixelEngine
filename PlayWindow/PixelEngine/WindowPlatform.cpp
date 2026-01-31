#include "WindowPlatform.h"
#include "PixelEngineAPI.h"
#include "PixelEngine.h"
#include <windows.h>
#include <process.h>
#include <iostream>

int WindowPlatform::width = 0;
int WindowPlatform::height = 0;
extern PixelEngine* Engine;
LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        PAINTSTRUCT pr;
        BeginPaint(hWnd, &pr);
        EndPaint(hWnd, &pr);
        break;
    case WM_SETFOCUS:
        break;
    case WM_KILLFOCUS:
        break;
    case WM_SIZE:
        WindowPlatform::width = LOWORD(lParam);
        WindowPlatform::height = HIWORD(lParam);
        break;
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


    //현재 모니터의 최대 크기 해상도
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);     // 현재 모니터 가로 해상도
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);    // 현재 모니터 세로 해상도

    WindowPlatform* pThis = (WindowPlatform*)pArguments;

    RegisterClass(&wc);
    pThis->g_hWnd = CreateWindowEx(0, L"PixelEngineClass", pThis->windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, wc.hInstance, NULL);

    MSG msg;
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                break;
            }
        }
    }

    return 0;
}

void WindowPlatform::Initialize()
{
    //윈도우 상태를 킨다.
    _beginthreadex(NULL, 0, WindowThread,this, 0, NULL);
}

void WindowPlatform::Update()
{
    

}

void WindowPlatform::Release()
{
    
}

void WindowPlatform::QuitWindow()
{
    PostQuitMessage(0);
}
