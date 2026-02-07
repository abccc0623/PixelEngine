#include "PixelEngineAPI.h"
#include <windows.h>
#include <functional>
#include <vector>
#include <string>
#include <iostream>

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
        //WindowPlatform::width = LOWORD(lParam);
        //WindowPlatform::height = HIWORD(lParam);
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int main() {
    // 2. 윈도우 클래스 등록 및 창 생성
    HINSTANCE hInst = GetModuleHandle(NULL);
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"MyEngineWindow";
    RegisterClass(&wc);

    HWND hWnd = CreateWindow(L"MyEngineWindow", L"My Game Engine", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInst, NULL);

    ShowWindow(hWnd, SW_SHOW);
    EngineInitialize(hWnd,800,600);

    LoadLuaScript("./Asset/Setting.Lua");

    MSG msg = { 0 };
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
        else
        {
            UpdateEngine();
        }
    }

    return (int)msg.wParam;
}
