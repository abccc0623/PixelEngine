using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using ICSharpCode.AvalonEdit;
using Newtonsoft.Json;

namespace PixelTool
{
    // Native hooks invoke this callback on the engine thread. Only immutable snapshots cross to WPF.
    internal static class LuaDebugSession
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int BreakCallback(IntPtr json);
        private static readonly BreakCallback Callback = OnBreak;
        private static bool registered;
        internal static bool Enabled { get; private set; }
        internal static bool IsInspecting { get; private set; }
        private static volatile bool shuttingDown;
        internal static bool IsShuttingDown => shuttingDown;
        internal static event Action? StateChanged;

        [DllImport("PixelEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void LuaDebug_SetCallback(BreakCallback callback);
        [DllImport("PixelEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void LuaDebug_SetEnabled(int enabled);
        [DllImport("PixelEngine.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void LuaDebug_SetBreakpoint([MarshalAs(UnmanagedType.LPUTF8Str)] string path, int line, int enabled);

        internal static bool SetEnabled(bool value)
        {
            try
            {
                if (!registered) { LuaDebug_SetCallback(Callback); registered = true; }
                LuaDebug_SetEnabled(value ? 1 : 0);
                Enabled = value;
                StateChanged?.Invoke();
                return true;
            }
            catch (Exception ex) when (ex is DllNotFoundException || ex is EntryPointNotFoundException || ex is BadImageFormatException)
            {
                PixelMessageBox.Show("Lua 디버거가 포함된 PixelEngine.dll을 먼저 빌드해 주세요.\n" + ex.Message,
                    "Lua 디버거", MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }
        }

        internal static bool SetBreakpoint(string path, int line, bool value)
        {
            if (!registered && !SetEnabled(true)) return false;
            LuaDebug_SetBreakpoint(path, line, value ? 1 : 0);
            return true;
        }

        internal static void Shutdown()
        {
            shuttingDown = true;
            if (registered) LuaDebug_SetEnabled(0);
        }

        private static int OnBreak(IntPtr json)
        {
            try
            {
                var snapshot = JsonConvert.DeserializeObject<DebugSnapshot>(Marshal.PtrToStringUTF8(json) ?? "{}");
                var dispatcher = Application.Current?.Dispatcher;
                if (snapshot == null || dispatcher == null || dispatcher.HasShutdownStarted || IsShuttingDown) return 2;
                var operation = dispatcher.InvokeAsync(() =>
                {
                    if (IsShuttingDown) return 2;
                    IsInspecting = true;
                    try
                    {
                        var inspector = new LuaBreakWindow(snapshot) { Owner = Application.Current?.MainWindow };
                        inspector.ShowDialog();
                        if (inspector.Command == 2) SetEnabled(false);
                        return inspector.Command;
                    }
                    finally { IsInspecting = false; }
                });
                while (!operation.Task.Wait(25))
                    if (IsShuttingDown || dispatcher.HasShutdownStarted) { operation.Abort(); return 2; }
                return operation.Task.Result;
            }
            catch { return 2; } // Never unwind managed exceptions into the Lua VM.
        }
    }

    internal sealed class DebugSnapshot
    {
        public List<DebugFrame> Frames { get; set; } = new();
        public bool Truncated { get; set; }
    }
    internal sealed class DebugFrame
    {
        public string Name { get; set; } = "";
        public string Source { get; set; } = "";
        public bool File { get; set; }
        public int Line { get; set; }
        public List<DebugVariable> Variables { get; set; } = new();
        public override string ToString() => $"{Name} — {Source}:{Line}";
    }
    internal sealed class DebugVariable
    {
        public string Name { get; set; } = "";
        public string Type { get; set; } = "";
        public string Value { get; set; } = "";
        public List<DebugVariable> Children { get; set; } = new();
    }

    internal sealed class LuaBreakWindow : Window
    {
        internal int Command { get; private set; } = 2;
        private readonly TextEditor source = new() { IsReadOnly = true, ShowLineNumbers = true,
            FontFamily = new FontFamily("Consolas"), FontSize = 14, Background = new SolidColorBrush(Color.FromRgb(32, 38, 48)),
            Foreground = Brushes.WhiteSmoke };
        private readonly TreeView variables = new() { Background = new SolidColorBrush(Color.FromRgb(32, 38, 48)), Foreground = Brushes.WhiteSmoke };
        private readonly TextBlock location = new() { TextWrapping = TextWrapping.Wrap, Margin = new Thickness(6) };
        private int selectedLine;

        internal LuaBreakWindow(DebugSnapshot snapshot)
        {
            Title = "Lua 중단점 — 실행 일시 정지";
            Width = 1100; Height = 700; MinWidth = 700; MinHeight = 400;
            WindowStartupLocation = WindowStartupLocation.CenterOwner;
            Background = new SolidColorBrush(Color.FromRgb(27, 32, 41)); Foreground = Brushes.WhiteSmoke;
            source.Options.HighlightCurrentLine = true;
            source.Loaded += (_, _) => { if (selectedLine > 0) source.ScrollToLine(selectedLine); };
            var root = new DockPanel { Margin = new Thickness(10) };
            Content = root;
            var controls = new StackPanel { Orientation = Orientation.Horizontal };
            AddButton(controls, "계속 실행 (F5)", 0);
            AddButton(controls, "다음 줄 진입 (F11)", 1);
            AddButton(controls, "디버깅 해제 후 계속 (Esc)", 2);
            DockPanel.SetDock(controls, Dock.Top); root.Children.Add(controls);
            var hint = new TextBlock { Margin = new Thickness(6), TextWrapping = TextWrapping.Wrap,
                Text = "호출 스택을 선택하면 해당 함수의 변수를 볼 수 있습니다. 테이블은 ▶로 펼치세요.\n소스는 현재 저장된 파일입니다. 실행 중인 코드와 맞추려면 수정 후 저장·재시작하세요."
                    + (snapshot.Truncated ? "\n변수가 많아 미리보기가 일부 생략되었습니다." : "") };
            DockPanel.SetDock(hint, Dock.Bottom); root.Children.Add(hint);
            var frames = new ListBox { ItemsSource = snapshot.Frames, Height = 100, Margin = new Thickness(0, 6, 0, 6),
                Background = source.Background, Foreground = Brushes.WhiteSmoke };
            DockPanel.SetDock(frames, Dock.Top); root.Children.Add(frames);
            DockPanel.SetDock(location, Dock.Top); root.Children.Add(location);
            var grid = new Grid();
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(3, GridUnitType.Star) });
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(5) });
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(2, GridUnitType.Star) });
            grid.Children.Add(source);
            var splitter = new GridSplitter { Width = 5, HorizontalAlignment = HorizontalAlignment.Stretch };
            Grid.SetColumn(splitter, 1); grid.Children.Add(splitter);
            Grid.SetColumn(variables, 2); grid.Children.Add(variables); root.Children.Add(grid);
            frames.SelectionChanged += (_, _) => { if (frames.SelectedItem is DebugFrame frame) ShowFrame(frame); };
            if (frames.Items.Count > 0) frames.SelectedIndex = 0;
            PreviewKeyDown += (_, e) =>
            {
                if (e.Key == Key.F5 || e.Key == Key.F11 || e.Key == Key.Escape)
                { e.Handled = true; Finish(e.Key == Key.F5 ? 0 : e.Key == Key.F11 ? 1 : 2); }
            };
        }

        private void AddButton(Panel panel, string text, int command)
        {
            var button = new Button { Content = text, Margin = new Thickness(3), Padding = new Thickness(10, 6, 10, 6) };
            button.Click += (_, _) => Finish(command);
            panel.Children.Add(button);
        }
        private void Finish(int command) { Command = command; Close(); }
        private void ShowFrame(DebugFrame frame)
        {
            selectedLine = 0;
            location.Text = $"{frame.Source} : {frame.Line}";
            variables.Items.Clear();
            foreach (var variable in frame.Variables) variables.Items.Add(CreateNode(variable));
            try
            {
                source.Text = frame.File && File.Exists(frame.Source) && new FileInfo(frame.Source).Length <= 2 * 1024 * 1024
                    ? File.ReadAllText(frame.Source) : "이 프레임의 소스 파일을 표시할 수 없습니다.";
                if (frame.File && frame.Line > 0 && frame.Line <= source.Document.LineCount)
                {
                    var line = source.Document.GetLineByNumber(frame.Line);
                    selectedLine = frame.Line;
                    source.Select(line.Offset, line.Length);
                    source.ScrollToLine(frame.Line);
                    source.TextArea.Caret.Line = frame.Line;
                    source.TextArea.TextView.CurrentLineBackground = new SolidColorBrush(Color.FromArgb(100, 230, 180, 40));
                }
            }
            catch (Exception ex) { source.Text = ex.Message; }
        }
        private static TreeViewItem CreateNode(DebugVariable value)
        {
            var item = new TreeViewItem { Header = $"{value.Name} = {value.Value}  ({value.Type})", Foreground = Brushes.WhiteSmoke };
            foreach (var child in value.Children) item.Items.Add(CreateNode(child));
            return item;
        }
    }
}
