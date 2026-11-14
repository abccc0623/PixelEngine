using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Interop;
using System.Windows;
using System.Collections.Concurrent;
using System.Collections.Generic;
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

        [DllImport("user32.dll")]
        static extern bool IsChild(IntPtr hWndParent, IntPtr hWnd);
        [DllImport("user32.dll")]
        private static extern IntPtr GetFocus();
        [DllImport("user32.dll")]
        private static extern IntPtr SetFocus(IntPtr hWnd);
        #endregion

        // Win32 상수
        private const int WS_CHILD = 0x40000000;
        private const int HOST_ID = 0x00000002;
        private const int WS_BORDER = 0x00800000;
        // Without SS_NOTIFY, STATIC returns HTTRANSPARENT: mouse clicks reach WPF,
        // so the native focus/keyboard isolation path is never activated.
        private const int SS_NOTIFY = 0x0100;

        // 엔진 제어 변수
        private IntPtr _childHwnd;
        internal bool HasNativeKeyboardFocus
        {
            get
            {
                IntPtr focus = GetFocus();
                return _childHwnd != IntPtr.Zero && (focus == _childHwnd || IsChild(_childHwnd, focus));
            }
        }

        // 멀티스레드 동기화 및 관리 (HyungSunGameEngine 최적화 포인트)
        private Thread _renderThread;
        private volatile bool _isRunning; // 스레드 가시성 보장
        private readonly object _engineLock = new object(); // C++ Native 호출 동기화 락
        private readonly ConcurrentQueue<Action> _pendingUiChanges = new ConcurrentQueue<Action>();
        private readonly ConcurrentQueue<Action> _afterFrameActions = new();
        private readonly SemaphoreSlim _captureLock = new(1, 1);
        private readonly List<(int Remaining, long Cursor, TaskCompletionSource<long> Completion, CancellationToken Token, long EndTicks, byte[] Keys)> _testKeys = new();

        [DllImport("PixelEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.I1)]
        private static extern bool Input_QueueVirtualKey(byte virtualKey, int frames);

        internal async Task<long> RequestTestKeyAsync(byte virtualKey, int frames, CancellationToken token)
            => await RequestTestKeysAsync([virtualKey], frames, token).ConfigureAwait(false);

        internal async Task<long> RequestTestKeysAsync(byte[] virtualKeys, int frames, CancellationToken token, int durationMs = 0)
        {
            if (!_isRunning) throw new InvalidOperationException("Scene View 엔진이 실행 중이 아닙니다.");
            if (frames is < 1 or > 120) throw new ArgumentOutOfRangeException(nameof(frames));
            if (durationMs != 0 && durationMs is < 100 or > 5000) throw new ArgumentOutOfRangeException(nameof(durationMs));
            if (virtualKeys.Length is < 1 or > 4 || virtualKeys.Distinct().Count() != virtualKeys.Length)
                throw new ArgumentException("서로 다른 키 1~4개가 필요합니다.", nameof(virtualKeys));
            var completion = new TaskCompletionSource<long>(TaskCreationOptions.RunContinuationsAsynchronously);
            _pendingUiChanges.Enqueue(() =>
            {
                if (token.IsCancellationRequested || !_isRunning) { completion.TrySetCanceled(token); return; }
                try
                {
                    long cursor = ConsoleWindow.LogCursor;
                    foreach (byte virtualKey in virtualKeys)
                        if (!Input_QueueVirtualKey(virtualKey, durationMs == 0 ? frames : 2))
                            throw new InvalidOperationException("게임 키 입력을 예약하지 못했습니다.");
                    long endTicks = durationMs == 0 ? 0 : System.Diagnostics.Stopwatch.GetTimestamp() +
                        (long)(durationMs * (System.Diagnostics.Stopwatch.Frequency / 1000.0));
                    _testKeys.Add((durationMs == 0 ? frames + 1 : 0, cursor, completion, token, endTicks, virtualKeys));
                }
                catch (Exception ex) { completion.TrySetException(ex); }
            });
            return await completion.Task.WaitAsync(token).ConfigureAwait(false);
        }

        internal async Task<(byte[] Png, int Width, int Height)> RequestGameFrameAsync(int maxWidth, CancellationToken token)
        {
            if (!_isRunning) throw new InvalidOperationException("Scene View 엔진이 실행 중이 아닙니다.");
            if (maxWidth is < 320 or > 1920) throw new ArgumentOutOfRangeException(nameof(maxWidth));
            using var deadline = CancellationTokenSource.CreateLinkedTokenSource(token);
            deadline.CancelAfter(TimeSpan.FromSeconds(15));
            await _captureLock.WaitAsync(deadline.Token).ConfigureAwait(false);
            try
            {
                var completion = new TaskCompletionSource<(byte[], int, int)>(TaskCreationOptions.RunContinuationsAsynchronously);
                _pendingUiChanges.Enqueue(() =>
                {
                    if (deadline.IsCancellationRequested || !_isRunning) { completion.TrySetCanceled(deadline.Token); return; }
                    try
                    {
                        if (!PixelEngineNative.EngineRequestFrameCapture(maxWidth))
                            throw new InvalidOperationException("게임 화면 캡처를 예약하지 못했습니다.");
                        _afterFrameActions.Enqueue(() =>
                        {
                            try
                            {
                                int size = PixelEngineNative.EngineCopyFrameCapture(null, 0, out int width, out int height);
                                if (size <= 0) throw new InvalidOperationException("게임 화면 캡처에 실패했습니다.");
                                byte[] png = new byte[size];
                                if (PixelEngineNative.EngineCopyFrameCapture(png, png.Length, out width, out height) != size)
                                    throw new InvalidOperationException("게임 화면 데이터를 읽지 못했습니다.");
                                completion.TrySetResult((png, width, height));
                            }
                            catch (Exception ex) { completion.TrySetException(ex); }
                        });
                    }
                    catch (Exception ex) { completion.TrySetException(ex); }
                });
                return await completion.Task.WaitAsync(deadline.Token).ConfigureAwait(false);
            }
            finally { _captureLock.Release(); }
        }

        internal async Task<long> RequestReimportAsync(CancellationToken token)
        {
            if (!_isRunning) throw new InvalidOperationException("Scene View 엔진이 실행 중이 아닙니다.");
            using var deadline = CancellationTokenSource.CreateLinkedTokenSource(token);
            deadline.CancelAfter(TimeSpan.FromSeconds(15));
            var cancelled = deadline.Token;
            var completion = new TaskCompletionSource<long>(TaskCreationOptions.RunContinuationsAsynchronously);
            _pendingUiChanges.Enqueue(() =>
            {
                if (cancelled.IsCancellationRequested || !_isRunning) { completion.TrySetCanceled(); return; }
                try
                {
                    long cursor = ConsoleWindow.LogCursor;
                    PixelEngineNative.Reload();
                    // Reload requests a clear; UpdateEngine below performs it and imports main.lua.
                    _afterFrameActions.Enqueue(() => completion.TrySetResult(cursor));
                }
                catch (Exception ex) { completion.TrySetException(ex); }
            });
            return await completion.Task.WaitAsync(cancelled).ConfigureAwait(false);
        }

        public GraphicsWindow()
        {
            Focusable = true;
            // 초기화 로직은 BuildWindowCore에서 수행
        }

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            int w = (int)ActualWidth > 0 ? (int)ActualWidth : 800;
            int h = (int)ActualHeight > 0 ? (int)ActualHeight : 600;

            // 1. 중계용 자식 창 생성
            _childHwnd = CreateWindowEx(
                0, "static", "EngineView", WS_CHILD | WS_BORDER | SS_NOTIFY,
                0, 0, w, h,
                hwndParent.Handle, (IntPtr)HOST_ID, IntPtr.Zero, 0);

            // 2. 엔진 초기화 (이 시점은 단일 스레드이므로 Lock 불필요)
            string root = ProjectPathService.ProjectRootPath.Replace('\\', '/');
            GlobalFunction.CreateMainLuaFile(ProjectPathService.AssetPath);
            PixelEngineNative.EngineInitialize(_childHwnd, w, h, root);
            PixelEngineNative.SetWindowFocus(false);
            PixelEngineNative.EditorNotify(EditorEventType.ToolInitialize, "");
            ProjectPathService.StartNewProject();
            PixelEngineNative.Asset_Import("./Asset/main.lua");

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
                    while (_pendingUiChanges.TryDequeue(out var change)) change();
                    PixelEngineNative.UpdateEngine();
                    while (_afterFrameActions.TryDequeue(out var finished)) finished();
                    for (int i = _testKeys.Count - 1; i >= 0; --i)
                    {
                        var pending = _testKeys[i];
                        if (pending.Token.IsCancellationRequested) { pending.Completion.TrySetCanceled(pending.Token); _testKeys.RemoveAt(i); continue; }
                        if (pending.EndTicks != 0 && pending.Remaining == 0)
                        {
                            if (System.Diagnostics.Stopwatch.GetTimestamp() < pending.EndTicks)
                            {
                                bool queued = true;
                                foreach (byte key in pending.Keys) queued &= Input_QueueVirtualKey(key, 2);
                                if (!queued) { pending.Completion.TrySetException(new InvalidOperationException("게임 키 입력을 유지하지 못했습니다.")); _testKeys.RemoveAt(i); }
                                continue;
                            }
                            _testKeys[i] = (3, pending.Cursor, pending.Completion, pending.Token, pending.EndTicks, pending.Keys);
                            continue;
                        }
                        if (pending.Remaining > 1) { _testKeys[i] = (pending.Remaining - 1, pending.Cursor, pending.Completion, pending.Token, pending.EndTicks, pending.Keys); continue; }
                        pending.Completion.TrySetResult(pending.Cursor);
                        _testKeys.RemoveAt(i);
                    }
                }
            }
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            base.OnRenderSizeChanged(sizeInfo);
            if (sizeInfo.NewSize.Width > 0 && sizeInfo.NewSize.Height > 0)
            {
                // UI 스레드에서 호출됨 -> 렌더 스레드와 충돌 방지를 위해 Lock
                int width = (int)sizeInfo.NewSize.Width;
                int height = (int)sizeInfo.NewSize.Height;
                _pendingUiChanges.Enqueue(() => PixelEngineNative.ResizeEngine(width, height));
            }
        }

        protected override void OnLostKeyboardFocus(System.Windows.Input.KeyboardFocusChangedEventArgs e)
        {
            _pendingUiChanges.Enqueue(() => PixelEngineNative.SetWindowFocus(false));
            base.OnLostKeyboardFocus(e);
        }

        private void OnThreadFilterMessage(ref MSG msg, ref bool handled)
        {
            if (LuaDebugSession.IsInspecting) return;
            // ThreadFilterMessage also sees WPF and ComboBox popup clicks.
            // Only the native render window (or its children) may take engine focus.
            if (_childHwnd == IntPtr.Zero ||
                (msg.hwnd != _childHwnd && !IsChild(_childHwnd, msg.hwnd))) return;
            // The game polls GetAsyncKeyState; WPF must not also use these messages
            // for GridSplitter resizing, tab navigation or another editor's caret.
            bool closeShortcut = (msg.message == 0x0104 || msg.message == 0x0105) && msg.wParam.ToInt64() == 0x73;
            if (msg.message >= 0x0100 && msg.message <= 0x0109 && !closeShortcut &&
                Application.Current.MainWindow is MainWindow main && main.IsSceneGameInputActive)
            {
                handled = true;
                return;
            }
            const int WM_LBUTTONDOWN = 0x0201;
            const int WM_RBUTTONDOWN = 0x0204;
            const int WM_MBUTTONDOWN = 0x0207;

            if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_MBUTTONDOWN)
            {
                this.Focus();
                SetFocus(_childHwnd);
                bool focused = HasNativeKeyboardFocus;
                _pendingUiChanges.Enqueue(() => PixelEngineNative.SetWindowFocus(focused));
            }
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            ComponentDispatcher.ThreadFilterMessage -= OnThreadFilterMessage;

            // 1. 렌더 스레드 우아한 종료 (Graceful Shutdown)
            _isRunning = false;
            LuaDebugSession.Shutdown();

            // 스레드가 현재 프레임을 마칠 때까지 최대 1초 대기 (좀비 스레드 방지)
            if (_renderThread != null && _renderThread.IsAlive)
            {
                _renderThread.Join(1000);
            }

            // Do not take _engineLock after a timed-out join. A stalled native
            // frame owns it, so reacquiring it would block WPF shutdown forever.
            // ReleaseEngine must not run concurrently with that frame either.
            DestroyWindow(hwnd.Handle);
        }
    }
}
