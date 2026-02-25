using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace PixelTool.EngineAPI
{
    internal class PixelEngineAPI
    {
        // DLL 이름 정의 (출력 경로 통합 시 이름만 적으면 됩니다)
        private const string DllName = "PixelEngine.dll";

        #region Engine Lifecycle
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool EngineInitialize(IntPtr hWnd, int width, int height);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void UpdateEngine();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool RunningEngineCheck();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void ReleaseEngine();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void QuitWindow();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool ResizeEngine(int width, int height);
        #endregion

        #region KeyInput
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetKeyDown(byte number);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetKeyUp(byte number);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetKey(byte number);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetMousePosition_X();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetMousePosition_Y();
        #endregion

        #region Time
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern float GetDeltaTime();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern double GetTotalTime();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetFPS();
        #endregion

        #region Lua & Resources
        // C++의 std::string& 대응: LPStr로 마샬링
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LoadLuaScript([MarshalAs(UnmanagedType.LPStr)] string path);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool LoadTexture([MarshalAs(UnmanagedType.LPStr)] string path);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool CreateLuaAPIPath([MarshalAs(UnmanagedType.LPStr)] string path);
        #endregion
    }
}
