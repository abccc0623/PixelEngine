using System.ComponentModel;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace PixelTool
{
    public partial class MainWindow : Window
    {
        private readonly DispatcherTimer _fpsTimer;
        private readonly AIChatWindow _aiChat;
        private readonly string _layoutFilePath;
        private readonly string _legacyLayoutPath;

        public MainWindow()
        {
            InitializeComponent();
            _fpsTimer = new DispatcherTimer(DispatcherPriority.Background) { Interval = TimeSpan.FromSeconds(1) };
            // Remove staging parents before the first HWND/layout is created.
            ContentStaging.Children.Clear();
            _aiChat = new AIChatWindow();
            dockManager.Register("SceneView", "Scene View", SceneViewContent);
            dockManager.Register("LuaEditor", "Lua Editor", LuaEditorContent);
            dockManager.Register("Asset", "Asset", AssetContent);
            dockManager.Register("Log", "Log", LogContent);
            dockManager.Register("AIChat", "자비스", _aiChat);
            string directory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "PixelTool");
            _layoutFilePath = Path.Combine(directory, "DockLayout.json");
            _legacyLayoutPath = Path.Combine(directory, "Layout.xml");
            InputManager.Current.PreProcessInput += MainWindow_PreProcessInput;
            Loaded += (_, _) =>
            {
                dockManager.Initialize(this, _layoutFilePath, _legacyLayoutPath);
                ShowAIChat(false, placeBesideLua: true);
            };
            Closing += MainWindow_Closing;
            Closed += (_, _) =>
            {
                InputManager.Current.PreProcessInput -= MainWindow_PreProcessInput;
                _fpsTimer.Stop();
                dockManager.Dispose();
                (LuaEditorContent.Child as LuaEditorWindow)?.DisposeLanguageService();
                _aiChat.DisposeChat();
            };
            _fpsTimer.Tick += (_, _) =>
            {
                float frameTimeMilliseconds = Math.Max(0.0f, PixelEngineNative.GetDeltaTime()) * 1000.0f;
                FpsTextBlock.Text = $"FPS: {PixelEngineNative.GetFPS()}  |  Frame: {frameTimeMilliseconds:F2} ms";
            };
            _fpsTimer.Start();
        }

        private void MainWindow_PreProcessInput(object sender, PreProcessInputEventArgs e)
        {
            bool closeShortcut = e.StagingItem.Input is KeyEventArgs closeKey &&
                closeKey.Key == Key.System && closeKey.SystemKey == Key.F4 && (Keyboard.Modifiers & ModifierKeys.Alt) != 0;
            if (IsSceneGameInputActive && !LuaDebugSession.IsInspecting && !closeShortcut &&
                (e.StagingItem.Input is KeyEventArgs or TextCompositionEventArgs))
            {
                // InputManager also covers floating dock windows on this UI thread.
                e.StagingItem.Input.Handled = true;
                e.Cancel();
                return;
            }
            if (e.StagingItem.Input is not KeyEventArgs keyEvent) return;
            if (keyEvent.RoutedEvent != Keyboard.PreviewKeyDownEvent) return;
            if (keyEvent.Key != Key.R || (Keyboard.Modifiers & ModifierKeys.Control) == 0) return;
            ConsolePanel.ClearLogs();
        }

        private void MainWindow_Closing(object? sender, CancelEventArgs e)
        {
            if (LuaEditorContent.Child is LuaEditorWindow editor && !editor.ConfirmCloseAll())
            {
                e.Cancel = true;
                return;
            }
            InputManager.Current.PreProcessInput -= MainWindow_PreProcessInput;
            try { dockManager.SaveLayout(_layoutFilePath); }
            catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
            {
                System.Diagnostics.Debug.WriteLine("Dock layout save: " + ex.Message);
            }
            dockManager.BeginShutdown();
        }

        internal async Task<System.Text.Json.JsonElement> ReimportForAiAsync(CancellationToken token)
        {
            Dispatcher.VerifyAccess();
            if ((LuaEditorContent.Child as LuaEditorWindow)?.HasUnsavedFiles == true)
                return System.Text.Json.JsonSerializer.SerializeToElement(new { reimportCompleted = false, reason = "저장하지 않은 Lua 편집 내용이 있습니다. 먼저 저장해 주세요." });
            if (LuaDebugSession.IsInspecting || LuaDebugSession.IsShuttingDown)
                return System.Text.Json.JsonSerializer.SerializeToElement(new { reimportCompleted = false, reason = "Lua 디버거가 중단 또는 종료 상태입니다. 계속 실행한 뒤 다시 요청하세요." });
            long cursor;
            try { cursor = await GraphicsWindow.RequestReimportAsync(token); }
            catch (OperationCanceledException) when (!token.IsCancellationRequested)
            {
                return System.Text.Json.JsonSerializer.SerializeToElement(new { reimportCompleted = false,
                    reason = "리임포트 완료 대기 시간이 초과되었습니다. 이미 시작된 초기화는 진행됐을 수 있으므로 성공으로 판단하거나 자동 재시도하지 말고 로그·디버거 상태를 확인하세요." });
            }
            return System.Text.Json.JsonSerializer.SerializeToElement(new
            {
                reimportCompleted = true, afterCursor = cursor, logs = ConsoleWindow.SummarizeLogsForAi(cursor),
                note = "리임포트 프레임 처리가 끝났습니다. 반환된 로그와 이후 로그를 확인하세요. 오류가 없는 전체 실행을 보장하지 않습니다."
            });
        }


        public void ResetDockLayout() => dockManager.ResetLayout();

        public void ShowAIChat(bool activate = true, bool placeBesideLua = false)
            => dockManager.ShowDocument("AIChat", activate);

        internal string HighlightLuaLines(string relativePath, int startLine, int endLine)
        {
            Dispatcher.VerifyAccess();
            if (string.IsNullOrWhiteSpace(relativePath) || Path.IsPathRooted(relativePath) || relativePath.Contains(':'))
                return "프로젝트 상대 경로가 필요합니다.";
            string root = Path.TrimEndingDirectorySeparator(Path.GetFullPath(ProjectPathService.ProjectRootPath));
            string path = Path.GetFullPath(Path.Combine(root, relativePath));
            if (!path.StartsWith(root + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase) ||
                !new[] { ".lua", ".pxm" }.Contains(Path.GetExtension(path), StringComparer.OrdinalIgnoreCase) ||
                !File.Exists(path)) return "프로젝트 안의 기존 Lua 파일(.lua, .pxm)을 지정하세요.";
            for (string? current = path; current != null && !current.Equals(root, StringComparison.OrdinalIgnoreCase);
                current = Path.GetDirectoryName(current))
                if ((File.GetAttributes(current) & FileAttributes.ReparsePoint) != 0)
                    return "링크 경로는 열 수 없습니다.";
            if (startLine < 1 || endLine < startLine || endLine - startLine >= 200)
                return "1~200줄 범위를 지정하세요.";
            if (LuaEditorContent.Child is not LuaEditorWindow editor) return "Lua 에디터를 찾을 수 없습니다.";
            dockManager.ShowDocument("LuaEditor", true);
            if (!editor.HighlightLines(path, startLine, endLine)) return "해당 줄을 찾을 수 없습니다. 파일 내용을 다시 확인하세요.";
            return $"표시됨: {Path.GetRelativePath(root, path)} {startLine}~{endLine}줄";
        }

        internal IEnumerable<FrameworkElement> DockContents => dockManager.Contents;
        internal bool IsSceneGameInputActive => EditorToolbar.IsPlayMode && GraphicsWindow.HasNativeKeyboardFocus;

        internal Action OpenFloatingTool(string title, FrameworkElement content, double width, double height)
            => dockManager.OpenTool(title, content, width, height);
    }
}
