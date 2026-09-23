using System;
using System.Collections.Generic;
using System.Windows;

namespace PixelTool
{
    public partial class LuaEditorWindow
    {
        private readonly Dictionary<string, ISet<int>> breakpointLines = new(StringComparer.OrdinalIgnoreCase);
        private readonly ISet<int> emptyBreakpoints = new HashSet<int>();
        private LuaBreakpointMargin breakpointMargin = null!;
        private ISet<int> CurrentBreakpoints => breakpointLines.TryGetValue(targetPath, out var lines) ? lines : emptyBreakpoints;

        private void InitializeDebugger()
        {
            breakpointMargin = new LuaBreakpointMargin(() => CurrentBreakpoints, ToggleBreakpoint);
            LuaEditor.TextArea.LeftMargins.Insert(0, breakpointMargin);
            LuaDebugSession.StateChanged += UpdateDebugStatus;
            UpdateDebugStatus();
        }
        private void UpdateDebugStatus()
        {
            DebugToggle.Content = LuaDebugSession.Enabled ? "● 디버깅 켜짐" : "○ 디버깅 꺼짐";
            breakpointMargin.InvalidateVisual();
        }
        private void ToggleDebugging(object sender, RoutedEventArgs e) => LuaDebugSession.SetEnabled(!LuaDebugSession.Enabled);
        private void ToggleBreakpoint(int line)
        {
            if (activeFile == null) return;
            if (!activeFile.Document.UndoStack.IsOriginalFile)
            {
                PixelMessageBox.Show("먼저 파일을 저장하고 코드를 다시 불러온 뒤 중단점을 설정해 주세요. (Ctrl+R)",
                    "Lua 중단점", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }
            if (!breakpointLines.TryGetValue(targetPath, out var lines))
                breakpointLines[targetPath] = lines = new HashSet<int>();
            bool add = !lines.Contains(line);
            if (add && !LuaDebugSession.Enabled && !LuaDebugSession.SetEnabled(true)) return;
            if (!LuaDebugSession.SetBreakpoint(targetPath, line, add)) return;
            if (add) lines.Add(line); else lines.Remove(line);
            breakpointMargin.InvalidateVisual();
        }
        private void ClearEditedBreakpoints()
        {
            if (!breakpointLines.TryGetValue(targetPath, out var lines) || lines.Count == 0) return;
            foreach (var line in lines) LuaDebugSession.SetBreakpoint(targetPath, line, false);
            lines.Clear();
            breakpointMargin.InvalidateVisual();
            ConsoleWindow.LogMessage("Lua 코드가 변경되어 해당 파일의 중단점을 해제했습니다. 저장·재시작 후 다시 설정해 주세요.", 0);
        }
    }
}
