
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
        public static extern bool RunningEngineCheck();

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern void ReleaseEngine();

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern void UpdateEngine();

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetKeyDown(byte number);

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetKeyUp(byte number);

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetKey(byte number);

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern float GetDeltaTime();

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern double GetTotalTime();

        [DllImport(DllPath, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetFPS();
    }
}
