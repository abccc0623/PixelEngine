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
        private System.Diagnostics.ProcessStartInfo? _projectRestart;

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
            dockManager.Register("AIChat", "AI 자비스", _aiChat);
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

        internal long ClearEditorLogsForAi()
        {
            ConsolePanel.ClearLogs();
            return ConsoleWindow.LogCursor;
        }

        private void MainWindow_Closing(object? sender, CancelEventArgs e)
        {
            if (!dockManager.ConfirmCloseTools()) { e.Cancel = true; return; }
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
            if (_projectRestart != null && Application.Current is App app)
                app.ProjectRestart = _projectRestart;
        }

        internal void ChangeProject()
        {
            if (_projectRestart != null) return;
            try
            {
                var restart = App.CreateProjectRestart();
                if (PixelMessageBox.Show("프로젝트 변경을 위해 에디터를 다시 시작하고 프로젝트 선택창을 엽니다.\n실행 중인 게임·AI 작업과 이미지 미리보기가 종료됩니다.\n계속하시겠습니까?",
                    "프로젝트 변경", MessageBoxButton.YesNo, MessageBoxImage.Question) != MessageBoxResult.Yes) return;
                _projectRestart = restart;
                Close(); // Reuse the existing unsaved-script and tool close guards.
            }
            catch (Exception exception) when (exception is IOException or InvalidOperationException or UnauthorizedAccessException)
            { PixelMessageBox.Show("프로젝트 변경을 시작하지 못했습니다.\n" + exception.Message, "프로젝트 변경"); }
            finally { _projectRestart = null; }
        }

        internal async Task<System.Text.Json.JsonElement> ReimportForAiAsync(CancellationToken token)
        {
            Dispatcher.VerifyAccess();
            if ((LuaEditorContent.Child as LuaEditorWindow)?.HasUnsavedFiles == true)
                return System.Text.Json.JsonSerializer.SerializeToElement(new { reimportCompleted = false, reason = "저장하지 않은 Lua 편집 내용이 있습니다. 먼저 저장해 주세요." });
            if (LuaDebugSession.IsInspecting || LuaDebugSession.IsShuttingDown)
                return System.Text.Json.JsonSerializer.SerializeToElement(new { reimportCompleted = false, reason = "Lua 디버거가 중단 또는 종료 상태입니다. 계속 실행한 뒤 다시 요청하세요." });
            long clearedThroughCursor = ClearEditorLogsForAi();
            long cursor;
            try { cursor = await GraphicsWindow.RequestReimportAsync(token); }
            catch (OperationCanceledException) when (!token.IsCancellationRequested)
            {
                return System.Text.Json.JsonSerializer.SerializeToElement(new { reimportCompleted = false,
                    reason = "리임포트 완료 대기 시간이 초과되었습니다. 이미 시작된 초기화는 진행됐을 수 있으므로 성공으로 판단하거나 자동 재시도하지 말고 로그·디버거 상태를 확인하세요." });
            }
            return System.Text.Json.JsonSerializer.SerializeToElement(new
            {
                reimportCompleted = true, logsCleared = true, clearedThroughCursor, afterCursor = cursor, logs = ConsoleWindow.SummarizeLogsForAi(cursor),
                note = "리임포트 프레임 처리가 끝났습니다. 반환된 로그와 이후 로그를 확인하세요. 오류가 없는 전체 실행을 보장하지 않습니다."
            });
        }

        internal async Task<System.Text.Json.JsonElement> TestGameKeyForAiAsync(string key, int frames, CancellationToken token)
        {
            Dispatcher.VerifyAccess();
            if (!EditorToolbar.IsPlayMode || LuaDebugSession.IsInspecting)
                throw new InvalidOperationException("게임 테스트는 플레이 모드이며 디버거가 중단되지 않았을 때만 가능합니다.");
            string normalized = key.Trim().ToUpperInvariant();
            byte virtualKey = normalized switch
            {
                "SPACE" => 0x20, "ENTER" => 0x0D, "LEFT" => 0x25, "UP" => 0x26,
                "RIGHT" => 0x27, "DOWN" => 0x28,
                _ when normalized.Length == 1 && normalized[0] is >= 'A' and <= 'Z' => (byte)normalized[0],
                _ => throw new ArgumentException("지원 키: A-Z, Space, Enter, Left, Up, Right, Down", nameof(key))
            };
            long cursor = await GraphicsWindow.RequestTestKeyAsync(virtualKey, frames, token);
            var page = ConsoleWindow.ReadLogsForAi(cursor, 100);
            var events = page.GetProperty("entries").EnumerateArray()
                .Where(entry => (entry.GetProperty("message").GetString() ?? "").StartsWith("[AI_TEST]", StringComparison.Ordinal))
                .Select(entry => new { sequence = entry.GetProperty("sequence").GetInt64(), message = entry.GetProperty("message").GetString() })
                .Take(20).ToArray();
            return System.Text.Json.JsonSerializer.SerializeToElement(new
            {
                key = normalized, frames, afterCursor = cursor, events, logs = ConsoleWindow.SummarizeLogsForAi(cursor)
            });
        }

        internal System.Text.Json.JsonElement SetEditorModeForAi(string mode)
        {
            Dispatcher.VerifyAccess();
            if (LuaDebugSession.IsInspecting || LuaDebugSession.IsShuttingDown)
                throw new InvalidOperationException("Lua 디버거가 중단 또는 종료 중입니다. 계속 실행한 뒤 모드를 변경하세요.");
            bool play = mode.ToLowerInvariant() switch
            {
                "play" => true,
                "editor" => false,
                _ => throw new ArgumentException("mode는 play 또는 editor여야 합니다.", nameof(mode))
            };
            bool before = EditorToolbar.IsPlayMode;
            bool actual = EditorToolbar.SetPlayMode(play);
            if (actual != play) throw new InvalidOperationException("요청한 에디터 모드로 전환하지 못했습니다.");
            return System.Text.Json.JsonSerializer.SerializeToElement(new
            {
                mode = actual ? "play" : "editor", changed = before != actual
            });
        }


        public void ShowWiki()
        {
            var wiki = DockContents.OfType<WikiWindow>().FirstOrDefault();
            if (wiki == null) { wiki = new WikiWindow(); OpenFloatingTool("LLM Wiki", wiki, 1000, 700); }
            dockManager.ActivateContent(wiki);
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

        internal void OpenSpriteEditor(string? path = null)
        {
            var editor = DockContents.OfType<SpriteEditor>().FirstOrDefault();
            if (editor == null)
            {
                editor = new SpriteEditor();
                OpenFloatingTool("Sprite Editor", editor, 1180, 760);
            }
            if (path != null) editor.OpenDocument(path);
            dockManager.ActivateContent(editor);
        }
    }
}
