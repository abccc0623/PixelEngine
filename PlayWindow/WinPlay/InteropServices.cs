
using System.Runtime.InteropServices;

namespace WinPlay
{
    class PixelEngine
    {
        // DLL 파일 경로가 "Library/PixelEngine.dll"이므로 폴더 구조와 일치해야 합니다.
        private const string DllPath = "Library/PixelEngine.dll";

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool EngineInitialize();

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern void ReleaseEngine();

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern void UpdateEngine();

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern void OnReSize(int X, int Y);

        // C++의 int*는 C#에서 out 또는 ref 키워드로 매핑하면 편리합니다.
        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern void GetWindowSize(out int X, out int Y);
    }
}
