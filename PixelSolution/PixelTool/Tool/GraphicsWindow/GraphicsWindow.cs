using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Interop;
using System.Windows;
// System.Windows.Media 제거됨 (CompositionTarget 사용 안 함)

namespace PixelTool
{
    internal class GraphicsWindow : HwndHost
    {
        #region Win32 API
        [DllImport("user32", EntryPoint = "CreateWindowEx", CharSet = CharSet.Unicode)]
        private static extern IntPtr CreateWindowEx(
            int extendedStyle, string className, string windowName, int style,
            int x, int y, int width, int height,
            IntPtr parentWindowHandle, IntPtr menuHandle, IntPtr instanceHandle,
            [MarshalAs(UnmanagedType.AsAny)] object parameter);

        [DllImport("user32.dll")]
        static extern bool DestroyWindow(IntPtr hWnd);
        #endregion

        // Win32 상수
        private const int WS_CHILD = 0x40000000;
        private const int HOST_ID = 0x00000002;
        private const int WS_BORDER = 0x00800000;

        // 엔진 제어 변수
        private IntPtr _childHwnd;

        // 멀티스레드 동기화 및 관리 (HyungSunGameEngine 최적화 포인트)
        private Thread _renderThread;
        private volatile bool _isRunning; // 스레드 가시성 보장
        private readonly object _engineLock = new object(); // C++ Native 호출 동기화 락

        public GraphicsWindow()
        {
            // 초기화 로직은 BuildWindowCore에서 수행
        }

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            int w = (int)ActualWidth > 0 ? (int)ActualWidth : 800;
            int h = (int)ActualHeight > 0 ? (int)ActualHeight : 600;

            // 1. 중계용 자식 창 생성
            _childHwnd = CreateWindowEx(
                0, "static", "EngineView", WS_CHILD | WS_BORDER,
                0, 0, w, h,
                hwndParent.Handle, (IntPtr)HOST_ID, IntPtr.Zero, 0);

            // 2. 엔진 초기화 (이 시점은 단일 스레드이므로 Lock 불필요)
            PixelEngineNative.EngineInitialize(_childHwnd, w, h);
            PixelEngineNative.Import("./Asset/main.lua");

            // 메시지 훅 등록
            ComponentDispatcher.ThreadFilterMessage += OnThreadFilterMessage;

            // 3. 전용 렌더 스레드 가동
            _isRunning = true;
            _renderThread = new Thread(EngineLoop)
            {
                Name = "PixelEngine_RenderThread",
                IsBackground = true // 메인 창 종료 시 백그라운드 스레드 자동 종료 보장
            };
            _renderThread.Start();

            return new HandleRef(this, _childHwnd);
        }

        /// <summary>
        /// 별도의 스레드에서 무한 루프로 돌아가는 핵심 렌더 루프입니다.
        /// </summary>
        private void EngineLoop()
        {
            while (_isRunning)
            {
                // UI 스레드에서 Resize나 Focus 변경 시 C++ 엔진에 동시 접근하는 것을 방지
                lock (_engineLock)
                {
                    PixelEngineNative.UpdateEngine();
                }
            }
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            base.OnRenderSizeChanged(sizeInfo);
            if (sizeInfo.NewSize.Width > 0 && sizeInfo.NewSize.Height > 0)
            {
                // UI 스레드에서 호출됨 -> 렌더 스레드와 충돌 방지를 위해 Lock
                lock (_engineLock)
                {
                    PixelEngineNative.ResizeEngine((int)sizeInfo.NewSize.Width, (int)sizeInfo.NewSize.Height);
                }
            }
        }

        protected override void OnLostKeyboardFocus(System.Windows.Input.KeyboardFocusChangedEventArgs e)
        {
            lock (_engineLock)
            {
                PixelEngineNative.SetWindowFocus(false);
            }
            base.OnLostKeyboardFocus(e);
        }

        private void OnThreadFilterMessage(ref MSG msg, ref bool handled)
        {
            const int WM_LBUTTONDOWN = 0x0201;
            const int WM_RBUTTONDOWN = 0x0204;
            const int WM_MBUTTONDOWN = 0x0207;

            if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_MBUTTONDOWN)
            {
                // 주의: DirectlyOver는 HitTest를 발생시켜 UI 스레드 부하를 유발할 수 있습니다.
                // 성능이 미세하게 떨어진다면 이 부분의 구조를 걷어내는 것을 권장합니다.
                var mouseOver = System.Windows.Input.Mouse.DirectlyOver;

                if (mouseOver is System.Windows.Controls.MenuItem ||
                    mouseOver is System.Windows.Controls.ContextMenu ||
                    System.Windows.Input.Keyboard.FocusedElement is System.Windows.Controls.MenuItem)
                {
                    return;
                }

                this.Focus();

                lock (_engineLock)
                {
                    PixelEngineNative.SetWindowFocus(true);
                }
            }
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            ComponentDispatcher.ThreadFilterMessage -= OnThreadFilterMessage;

            // 1. 렌더 스레드 우아한 종료 (Graceful Shutdown)
            _isRunning = false;

            // 스레드가 현재 프레임을 마칠 때까지 최대 1초 대기 (좀비 스레드 방지)
            if (_renderThread != null && _renderThread.IsAlive)
            {
                _renderThread.Join(1000);
            }

            // 2. 엔진 리소스 해제
            lock (_engineLock)
            {
                //PixelEngineNative.ReleaseEngine();
            }

            // 3. 창 핸들 파괴
            DestroyWindow(hwnd.Handle);
        }
    }
}