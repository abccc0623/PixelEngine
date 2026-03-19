using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System;
using System.Runtime.InteropServices;

public static class PixelEngineNative
{
    // DLL 파일 이름에 맞게 수정하세요 (확장자 .dll은 생략 가능)
    private const string DllName = "PixelEngine.dll";

    // CallingConvention.Cdecl은 C++의 extern "C" 기본 호출 규약입니다.

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)] // C++ bool(1바이트)을 C# bool로 정확히 매핑
    public static extern bool EngineInitialize(IntPtr hWnd, int width, int height);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void UpdateEngine();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void ReleaseEngine();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void QuitWindow();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool ResizeEngine(int width, int height);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool LoadLuaFile([MarshalAs(UnmanagedType.LPStr)] string path);

    [DllImport("PixelEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetWindowFocus([MarshalAs(UnmanagedType.I1)] bool focus);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Reload();
}