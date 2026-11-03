using AvalonDock.Layout.Serialization;
using AvalonDock.Layout;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;

namespace PixelTool
{
    public partial class MainWindow : Window
    {
        private readonly Brush _dockBorderBrush = new SolidColorBrush(System.Windows.Media.Color.FromRgb(0x3A, 0x3C, 0x5E));
        private readonly Brush _dockBackgroundBrush = new SolidColorBrush(System.Windows.Media.Color.FromRgb(0x1B, 0x1C, 0x2E));
        private readonly Brush _dockTextBrush = new SolidColorBrush(System.Windows.Media.Color.FromRgb(0xD1, 0xD1, 0xE0));
        private readonly Brush _activeDockBackgroundBrush = new SolidColorBrush(Color.FromRgb(37, 74, 112));
        private readonly Brush _activeDockBorderBrush = new SolidColorBrush(Color.FromRgb(101, 185, 255));
        private readonly DispatcherTimer _fpsTimer;
        private LayoutRoot? _observedDockLayout;
        private bool _dockThemeQueued;
        private bool _closed;
        private readonly Dictionary<string, object> _layoutContents;
        private readonly string _layoutFilePath;
        private readonly string _defaultLayout;
        private readonly AIChatWindow _aiChat;

        public MainWindow()
        {
            InitializeComponent();
			_aiChat = new AIChatWindow();
			InputManager.Current.PreProcessInput += MainWindow_PreProcessInput;

            _layoutContents = new Dictionary<string, object>
            {
                ["SceneView"] = SceneViewContent,
                ["LuaEditor"] = LuaEditorContent,
                ["Asset"] = AssetContent,
                ["Log"] = LogContent,
                ["AIChat"] = _aiChat
            };
            _layoutFilePath = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                "PixelTool",
                "Layout.xml");
            _defaultLayout = SerializeCurrentLayout();
            dockManager.LayoutChanged += DockLayoutChanged;
            dockManager.ActiveContentChanged += DockThemeStateChanged;
            dockManager.LayoutFloatingWindowControlCreated += DockThemeStateChanged;
            dockManager.LayoutFloatingWindowControlClosed += DockThemeStateChanged;
            ObserveDockLayout();

            IntPtr hWnd = new WindowInteropHelper(this).Handle;
            Loaded += (_, _) =>
            {
                LoadSavedLayout();
                ShowAIChat(false, placeBesideLua: true);
                QueueDockThemeUpdate();
            };
            Closing += MainWindow_Closing;
            Closed += (_, _) =>
            {
                _closed = true;
                _fpsTimer.Stop();
                dockManager.LayoutChanged -= DockLayoutChanged;
                dockManager.ActiveContentChanged -= DockThemeStateChanged;
                dockManager.LayoutFloatingWindowControlCreated -= DockThemeStateChanged;
                dockManager.LayoutFloatingWindowControlClosed -= DockThemeStateChanged;
                DetachDockLayout();
                (LuaEditorContent.Child as LuaEditorWindow)?.DisposeLanguageService();
                _aiChat.DisposeChat();
            };

            _fpsTimer = new DispatcherTimer(DispatcherPriority.Background)
            {
                Interval = TimeSpan.FromMilliseconds(1000)
            };
            _fpsTimer.Tick += (_, _) =>
            {
                float frameTimeMilliseconds =
                    Math.Max(0.0f, PixelEngineNative.GetDeltaTime()) * 1000.0f;
                FpsTextBlock.Text =
                    $"FPS: {PixelEngineNative.GetFPS()}  |  Frame: {frameTimeMilliseconds:F2} ms";
            };
            _fpsTimer.Start();
        }

        private void DockLayoutChanged(object? sender, EventArgs e)
        {
            ObserveDockLayout();
            QueueDockThemeUpdate();
        }

        private void ObserveDockLayout()
        {
            DetachDockLayout();
            _observedDockLayout = dockManager.Layout;
            if (_observedDockLayout != null)
            {
                _observedDockLayout.ElementAdded += DockThemeStateChanged;
                _observedDockLayout.ElementRemoved += DockThemeStateChanged;
            }
        }

        private void DetachDockLayout()
        {
            if (_observedDockLayout == null) return;
            _observedDockLayout.ElementAdded -= DockThemeStateChanged;
            _observedDockLayout.ElementRemoved -= DockThemeStateChanged;
        }

        private void DockThemeStateChanged(object? sender, EventArgs e) => QueueDockThemeUpdate();

        private void QueueDockThemeUpdate()
        {
            if (_closed || _dockThemeQueued) return;
            _dockThemeQueued = true;
            // Model geometry updates fire continuously during docking drags. Do
            // not subscribe to Updated, and let input/rendering run before styling.
            Dispatcher.BeginInvoke(DispatcherPriority.Background, new Action(() =>
            {
                _dockThemeQueued = false;
                if (!_closed) ApplyAvalonDockRuntimeTheme();
            }));
        }

		private void MainWindow_PreProcessInput(object sender, PreProcessInputEventArgs e)
		{
			if (e.StagingItem.Input is not KeyEventArgs keyEvent) return;
			if (keyEvent.RoutedEvent != Keyboard.PreviewKeyDownEvent) return;
			if (keyEvent.Key != Key.R || (Keyboard.Modifiers & ModifierKeys.Control) == 0) return;
			ConsolePanel.ClearLogs();
		}

        private void MainWindow_Closing(object sender, CancelEventArgs e)
        {
            var editor = LuaEditorContent.Child as LuaEditorWindow;
            if (editor != null && !editor.ConfirmCloseAll()) { e.Cancel = true; return; }
			InputManager.Current.PreProcessInput -= MainWindow_PreProcessInput;
            SaveCurrentLayout();
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
                reimportCompleted = true, afterCursor = cursor, logs = ConsoleWindow.ReadLogsForAi(cursor, 100),
                note = "리임포트 프레임 처리가 끝났습니다. 반환된 로그와 이후 로그를 확인하세요. 오류가 없는 전체 실행을 보장하지 않습니다."
            });
        }

        private string SerializeCurrentLayout()
        {
            var serializer = new XmlLayoutSerializer(dockManager);
            using var writer = new StringWriter();
            serializer.Serialize(writer);
            return writer.ToString();
        }

        private void DeserializeLayout(string layoutXml)
        {
            var serializer = new XmlLayoutSerializer(dockManager);
            serializer.LayoutSerializationCallback += (_, e) =>
            {
                if (e.Model?.ContentId != null && _layoutContents.TryGetValue(e.Model.ContentId, out object content))
                {
                    e.Content = content;
                }
                else
                {
                    e.Cancel = true;
                }
            };

            using var reader = new StringReader(layoutXml);
            serializer.Deserialize(reader);
        }

        private void LoadSavedLayout()
        {
            if (!File.Exists(_layoutFilePath)) return;

            try
            {
                DeserializeLayout(File.ReadAllText(_layoutFilePath));
            }
            catch
            {
                DeserializeLayout(_defaultLayout);
            }
        }

        private void SaveCurrentLayout()
        {
            try
            {
                string directory = Path.GetDirectoryName(_layoutFilePath);
                if (!string.IsNullOrEmpty(directory)) Directory.CreateDirectory(directory);
                File.WriteAllText(_layoutFilePath, SerializeCurrentLayout());
            }
            catch
            {
                // Layout persistence must not prevent the editor from closing.
            }
        }

        public void ResetDockLayout()
        {
            DeserializeLayout(_defaultLayout);
            ShowAIChat(false, placeBesideLua: true);
            QueueDockThemeUpdate();

            try
            {
                if (File.Exists(_layoutFilePath)) File.Delete(_layoutFilePath);
            }
            catch
            {
            }
        }

        public void ShowAIChat(bool activate = true, bool placeBesideLua = false)
        {
            var existing = dockManager.Layout.Descendents().OfType<LayoutDocument>()
                .FirstOrDefault(document => document.ContentId == "AIChat");
            if (existing != null && !placeBesideLua)
            {
                existing.IsSelected = true;
                if (activate) existing.IsActive = true;
                return;
            }
            var document = existing ?? new LayoutDocument { Title = "AI Chat", ContentId = "AIChat", Content = _aiChat };
            var lua = dockManager.Layout.Descendents().OfType<LayoutDocument>()
                .FirstOrDefault(item => item.ContentId == "LuaEditor");
            if (lua?.Parent is LayoutDocumentPane luaPane && luaPane.Parent is LayoutPanel panel)
            {
                var oldPane = document.Parent as LayoutDocumentPane;
                if (oldPane != luaPane && oldPane?.Children.Count == 1 && oldPane.Parent == panel &&
                    panel.Orientation == Orientation.Horizontal && panel.Children.IndexOf(oldPane) == panel.Children.IndexOf(luaPane) + 1)
                {
                    lua.IsSelected = document.IsSelected = true;
                    if (activate) document.IsActive = true;
                    return;
                }
                document.Parent?.RemoveChild(document);
                if (panel.Orientation != Orientation.Horizontal)
                {
                    int index = panel.Children.IndexOf(luaPane);
                    var row = new LayoutPanel { Orientation = Orientation.Horizontal, DockWidth = luaPane.DockWidth, DockHeight = luaPane.DockHeight };
                    panel.Children.Remove(luaPane);
                    panel.Children.Insert(index, row);
                    row.Children.Add(luaPane);
                    panel = row;
                }
                var pane = new LayoutDocumentPane { DockWidth = new GridLength(0.65, GridUnitType.Star) };
                panel.Children.Insert(panel.Children.IndexOf(luaPane) + 1, pane);
                pane.Children.Add(document);
                lua.IsSelected = true;
                if (oldPane?.Children.Count == 0) oldPane.Parent?.RemoveChild(oldPane);
            }
            else if (lua?.Parent is LayoutDocumentPane groupedLua && groupedLua.Parent is LayoutDocumentPaneGroup group)
            {
                var oldPane = document.Parent as LayoutDocumentPane;
                document.Parent?.RemoveChild(document);
                var pane = new LayoutDocumentPane { DockWidth = new GridLength(0.65, GridUnitType.Star) };
                if (group.Orientation == Orientation.Horizontal)
                    group.Children.Insert(group.Children.IndexOf(groupedLua) + 1, pane);
                else
                {
                    int index = group.Children.IndexOf(groupedLua);
                    var row = new LayoutDocumentPaneGroup { Orientation = Orientation.Horizontal, DockWidth = groupedLua.DockWidth, DockHeight = groupedLua.DockHeight };
                    group.Children.Remove(groupedLua);
                    group.Children.Insert(index, row);
                    row.Children.Add(groupedLua);
                    row.Children.Add(pane);
                }
                pane.Children.Add(document);
                lua.IsSelected = true;
                if (oldPane?.Children.Count == 0) oldPane.Parent?.RemoveChild(oldPane);
            }
            else
            {
                if (existing != null) { existing.IsSelected = true; if (activate) existing.IsActive = true; return; }
                var pane = dockManager.Layout.Descendents().OfType<LayoutDocumentPane>().FirstOrDefault();
                if (pane == null) return;
                pane.Children.Add(document);
            }
            document.IsSelected = true;
            if (activate) document.IsActive = true;
            QueueDockThemeUpdate();
        }

        private void ApplyAvalonDockRuntimeTheme()
        {
            ApplyDockVisualTree(dockManager, false);

            foreach (Window window in Application.Current.Windows)
            {
                if (window == this) continue;
                if (window.GetType().FullName?.Contains("AvalonDock") != true) continue;

                window.Background = _dockBackgroundBrush;
                ApplyDockVisualTree(window, true);
            }
        }

        private void ApplyDockVisualTree(DependencyObject target, bool forceDockScope)
        {
            // Embedded editors own their tab/focus colors. Do not overwrite their styles.
            if (target is UserControl) return;
            bool isDockScope = forceDockScope || IsAvalonDockVisual(target);

            if (isDockScope)
            {
                if (target is Border border)
                {
                    border.BorderBrush = _dockBorderBrush;
                    if (IsLightBrush(border.Background)) border.Background = _dockBackgroundBrush;
                }
                else if (target is Control control)
                {
                    if (IsLightBrush(control.BorderBrush)) control.BorderBrush = _dockBorderBrush;
                    if (IsLightBrush(control.Background)) control.Background = _dockBackgroundBrush;
                    if (control is AvalonDock.Controls.LayoutDocumentTabItem documentTab)
                    {
                        bool active = documentTab.Model?.IsActive == true;
                        control.SetCurrentValue(Control.BackgroundProperty, active ? _activeDockBackgroundBrush : _dockBackgroundBrush);
                        control.SetCurrentValue(Control.ForegroundProperty, active ? Brushes.White : _dockTextBrush);
                        control.SetCurrentValue(Control.BorderBrushProperty, active ? _activeDockBorderBrush : _dockBorderBrush);
                    }
                }
                else if (target is Panel panel)
                {
                    if (IsLightBrush(panel.Background)) panel.Background = _dockBackgroundBrush;
                }
            }

            int count = VisualTreeHelper.GetChildrenCount(target);
            for (int i = 0; i < count; i++)
            {
                ApplyDockVisualTree(VisualTreeHelper.GetChild(target, i), isDockScope);
            }
        }

        private static bool IsAvalonDockVisual(DependencyObject target)
        {
            return target.GetType().FullName?.Contains("AvalonDock") == true;
        }

        private static bool IsAvalonDockTab(Control control)
        {
            string name = control.GetType().Name;
            return name.Contains("TabItem") || name.Contains("LayoutDocument");
        }

        private static bool IsLightBrush(Brush brush)
        {
            if (brush == null) return true;
            if (brush is not SolidColorBrush solid) return false;

            var color = solid.Color;
            return color.R > 180 && color.G > 180 && color.B > 180;
        }
    }
}
