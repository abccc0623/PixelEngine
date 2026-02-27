using System.Runtime.InteropServices;
using System.Windows.Interop;
using System.Windows.Media; // CompositionTarget 사용을 위해 필요

namespace PixelTool
{
    internal class GraphicsWindow : HwndHost
    {
        [DllImport("user32", EntryPoint = "CreateWindowEx", CharSet = CharSet.Unicode)]
        private static extern IntPtr CreateWindowEx
        (
            int extendedStyle,
            string className,
            string windowName,
            int style,
            int x,
            int y,
            int width,
            int height,
            IntPtr parentWindowHandle,
            IntPtr menuHandle,
            IntPtr instanceHandle,
            [MarshalAs(UnmanagedType.AsAny)] object parameter
        );

        [DllImport("user32.dll")]
        static extern bool DestroyWindow(IntPtr hWnd);

        public GraphicsWindow()
        {
            // 상용 엔진의 'Game Loop'처럼 매 프레임마다 Update를 호출하기 위해 등록
        }

        private const int WS_CHILD = 0x40000000;
        private const int HOST_ID = 0x00000002;
        private const int WS_BORDER = 0x00800000;
        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            // 1. 중계용 자식 창 생성 (WS_CHILD | WS_VISIBLE)
            // 실제 Width/Height가 0일 경우를 대비해 기본값을 주거나 ActualWidth를 사용합니다.
            int w = (int)ActualWidth > 0 ? (int)ActualWidth : 800;
            int h = (int)ActualHeight > 0 ? (int)ActualHeight : 600;

            var childHwnd = CreateWindowEx
            (
                0,
                "static",
                "test",
                WS_CHILD | WS_BORDER,
                0,
                0,
                w,
                h,
                hwndParent.Handle,
                (IntPtr)HOST_ID,
                IntPtr.Zero,
                0
            );

            // 2. 이 자식 창 핸들을 PixelEngine에 전달하여 초기화
            PixelEngine.EngineInitialize(childHwnd, w, h);
            PixelEngine.LoadLuaScript("./Asset/Setting.Lua");
            PixelEngine.CreateLuaAPIPath("./Asset/PixelEngine_API.lua");
            CompositionTarget.Rendering += OnRender;
            // 3. 자식 창 핸들을 HandleRef로 감싸서 반환 (에러 방지 핵심)
            return new HandleRef(this, childHwnd);
        }

        private void OnRender(object sender, EventArgs e)
        {
            PixelEngine.UpdateEngine();
        }

        protected override void OnRenderSizeChanged(System.Windows.SizeChangedInfo sizeInfo)
        {
            base.OnRenderSizeChanged(sizeInfo);
            if (sizeInfo.NewSize.Width > 0 && sizeInfo.NewSize.Height > 0)
            {
                PixelEngine.ResizeEngine((int)sizeInfo.NewSize.Width, (int)sizeInfo.NewSize.Height);
            }
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            // 렌더링 이벤트 해제
            CompositionTarget.Rendering -= OnRender;
            // 엔진 자원 해제
            PixelEngine.ReleaseEngine();
            // 생성한 Win32 창 파괴
            DestroyWindow(hwnd.Handle);
        }
    }
}