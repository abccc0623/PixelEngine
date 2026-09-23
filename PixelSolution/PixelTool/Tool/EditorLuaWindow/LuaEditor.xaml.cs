using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.CodeCompletion;
using ICSharpCode.AvalonEdit.Document;
using ICSharpCode.AvalonEdit.Editing;
using ICSharpCode.AvalonEdit.Highlighting;
using ICSharpCode.AvalonEdit.Highlighting.Xshd;
using ICSharpCode.AvalonEdit.Rendering;
using ICSharpCode.AvalonEdit.Search;
using Microsoft.VisualStudio.LanguageServer.Protocol;
using PixelTool;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace PixelTool
{
    public partial class LuaEditorWindow : UserControl
    {
        private string targetPath = "";
        private bool IsDirty = false;
        private sealed class FileState
        {
            public string Path;
            public TextDocument Document;
            public TabItem Tab;
            public TextBlock Label;
            public int Caret;
            public int SelectionStart;
            public int SelectionLength;
            public double VerticalOffset;
            public double HorizontalOffset;
        }
        private readonly Dictionary<string, FileState> files = new Dictionary<string, FileState>(StringComparer.OrdinalIgnoreCase);
        private FileState activeFile;
        private bool switchingFile;
        private readonly Task lspReady;
        public string CurrentFilePath => targetPath;

        public CompletionWindow completionWindow;
        private CancellationTokenSource _debounceTokenSource;
        private Dictionary<string, string> variableTypes = new Dictionary<string, string>();
        private LuaLspService luaLspService;
        private LuaDiagnosticRenderer diagnosticRenderer;
        private bool isWaitingForFormatChord = false;

        public LuaEditorWindow()
        {
            InitializeComponent();
            luaLspService = new LuaLspService();
            luaLspService.DiagnosticsPublished += LuaLspService_DiagnosticsPublished;
            lspReady = luaLspService.Initialize();

            ConfigureEditorOptions();
            InitializeDebugger();
            diagnosticRenderer = new LuaDiagnosticRenderer(LuaEditor.TextArea.TextView);
            LuaEditor.TextArea.TextView.BackgroundRenderers.Add(diagnosticRenderer);
            LuaEditor.TextChanged += LuaEditor_TextChanged;
            LuaEditor.TextArea.TextEntered += TextArea_TextEntered;
            LuaEditor.PreviewMouseWheel += LuaEditor_PreviewMouseWheel;
            ApplyLuaSyntaxHighlighting();
            LuaEditor.IsReadOnly = true;
        }

        private void LuaLspService_DiagnosticsPublished(PublishDiagnosticParams parameters)
        {
            Dispatcher.InvokeAsync(() =>
            {
                if (LuaEditor.Document == null || string.IsNullOrEmpty(targetPath)) return;

                string currentUri = new Uri(Path.GetFullPath(targetPath)).AbsoluteUri;
                if (!string.Equals(parameters.Uri.AbsoluteUri, currentUri, StringComparison.OrdinalIgnoreCase)) return;

                var errorSegments = new List<ISegment>();
                foreach (Diagnostic diagnostic in parameters.Diagnostics)
                {
                    if (diagnostic.Severity != DiagnosticSeverity.Error) continue;

                    int startOffset = GetDocumentOffset(diagnostic.Range.Start);
                    int endOffset = GetDocumentOffset(diagnostic.Range.End);
                    if (endOffset <= startOffset)
                    {
                        endOffset = Math.Min(LuaEditor.Document.TextLength, startOffset + 1);
                    }

                    if (endOffset > startOffset)
                    {
                        errorSegments.Add(new LuaDiagnosticSegment(startOffset, endOffset - startOffset));
                    }
                }

                diagnosticRenderer.SetErrors(errorSegments);
            });
        }

        private int GetDocumentOffset(Position position)
        {
            int lineNumber = Math.Max(1, Math.Min(LuaEditor.Document.LineCount, position.Line + 1));
            DocumentLine line = LuaEditor.Document.GetLineByNumber(lineNumber);
            int column = Math.Max(0, Math.Min(line.Length, position.Character));
            return line.Offset + column;
        }

        private void ConfigureEditorOptions()
        {
            LuaEditor.Options.ConvertTabsToSpaces = true;
            LuaEditor.Options.IndentationSize = 4;
            LuaEditor.Options.EnableRectangularSelection = true;
            LuaEditor.TextArea.TextView.CurrentLineBackground = new SolidColorBrush(System.Windows.Media.Color.FromRgb(42, 42, 42));
            LuaEditor.TextArea.TextView.CurrentLineBorder = new Pen(new SolidColorBrush(System.Windows.Media.Color.FromRgb(64, 64, 64)), 1);
            SearchPanel.Install(LuaEditor);
        }

        private void ApplyLuaSyntaxHighlighting()
        {
            try
            {
                var resourceName = "PixelTool.Tool.EditorLuaWindow.Lua.xshd";
                using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
                {
                    if (stream == null) return;
                    using (var reader = new System.Xml.XmlTextReader(stream))
                    {
                        LuaEditor.SyntaxHighlighting = HighlightingLoader.Load(reader, HighlightingManager.Instance);
                    }
                }
            }
            catch (Exception ex)
            {
                ConsoleWindow.LogMessage("Highlighting Load Failed: " + ex.Message, 2);
            }
        }

        public void OpenFile(string path)
        {
            try
            {
                path = Path.GetFullPath(path);
                if (!File.Exists(path)) return;
                if (!files.TryGetValue(path, out var file))
                {
                    file = new FileState { Path = path, Document = new TextDocument(File.ReadAllText(path)) };
                    file.Document.UndoStack.MarkAsOriginalFile();
                    file.Document.UndoStack.PropertyChanged += (_, args) =>
                    {
                        if (args.PropertyName == nameof(UndoStack.IsOriginalFile)) Dispatcher.InvokeAsync(UpdateFileStatus);
                    };
                    file.Label = new TextBlock { Text = Path.GetFileName(path), VerticalAlignment = VerticalAlignment.Center };
                    var header = new StackPanel { Orientation = Orientation.Horizontal };
                    header.Children.Add(file.Label);
                    var close = new Button { Content = "×", Margin = new Thickness(8, 0, 0, 0), Padding = new Thickness(4, 0, 4, 0), ToolTip = "Close file" };
                    var captured = file;
                    close.Click += (_, e) => { e.Handled = true; CloseFile(captured); };
                    header.Children.Add(close);
                    file.Tab = new TabItem { Header = header, Tag = file, ToolTip = path };
                    files.Add(path, file);
                    FileTabs.Items.Add(file.Tab);
                }
                FileTabs.SelectedItem = file.Tab;
                LuaEditor.Focus();
            }
            catch (Exception ex) { PixelMessageBox.Show(ex.Message, "파일 열기 실패", MessageBoxButton.OK, MessageBoxImage.Error); }
        }

        private async void FileTabs_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.Source != FileTabs) return;
            completionWindow?.Close();
            if (activeFile != null)
            {
                activeFile.Caret = LuaEditor.CaretOffset;
                activeFile.SelectionStart = LuaEditor.SelectionStart;
                activeFile.SelectionLength = LuaEditor.SelectionLength;
                activeFile.VerticalOffset = LuaEditor.VerticalOffset;
                activeFile.HorizontalOffset = LuaEditor.HorizontalOffset;
            }
            switchingFile = true;
            activeFile = (FileTabs.SelectedItem as TabItem)?.Tag as FileState;
            targetPath = activeFile?.Path ?? "";
            LuaEditor.Document = activeFile?.Document ?? new TextDocument();
            LuaEditor.IsReadOnly = activeFile == null;
            LuaEditor.CaretOffset = Math.Min(activeFile?.Caret ?? 0, LuaEditor.Document.TextLength);
            if (activeFile != null) LuaEditor.Select(activeFile.SelectionStart, activeFile.SelectionLength);
            LuaEditor.ScrollToVerticalOffset(activeFile?.VerticalOffset ?? 0);
            LuaEditor.ScrollToHorizontalOffset(activeFile?.HorizontalOffset ?? 0);
            diagnosticRenderer.SetErrors(new List<ISegment>());
            switchingFile = false;
            UpdateFileStatus();
            var selected = activeFile;
            await lspReady;
            if (selected != null && selected == activeFile)
                await luaLspService.NotifyFileOpenAsync(selected.Path, selected.Document.Text);
        }

        private void UpdateFileStatus()
        {
            IsDirty = activeFile != null && !activeFile.Document.UndoStack.IsOriginalFile;
            if (activeFile != null) activeFile.Label.Text = Path.GetFileName(targetPath) + (IsDirty ? "*" : "");
            EditorChange.Text = IsDirty ? "● Modified" : "● Saved";
            EditorChange.Foreground = new SolidColorBrush(IsDirty ? System.Windows.Media.Color.FromRgb(244, 190, 74) : System.Windows.Media.Color.FromRgb(99, 193, 116));
        }

        private bool SaveFile(FileState file)
        {
            if (file == null) return false;
            try
            {
                File.WriteAllText(file.Path, file.Document.Text);
                file.Document.UndoStack.MarkAsOriginalFile();
                file.Label.Text = Path.GetFileName(file.Path);
                UpdateFileStatus();
                return true;
            }
            catch (Exception ex) { PixelMessageBox.Show(ex.Message, "저장 실패", MessageBoxButton.OK, MessageBoxImage.Error); return false; }
        }

        private bool ConfirmClose(FileState file)
        {
            if (file.Document.UndoStack.IsOriginalFile) return true;
            var answer = PixelMessageBox.Show(Path.GetFileName(file.Path) + " 변경 사항을 저장할까요?", "저장", MessageBoxButton.YesNoCancel, MessageBoxImage.Question);
            return answer == MessageBoxResult.No || (answer == MessageBoxResult.Yes && SaveFile(file));
        }

        private async void CloseFile(FileState file)
        {
            if (!ConfirmClose(file)) return;
            files.Remove(file.Path);
            FileTabs.Items.Remove(file.Tab);
            await lspReady;
            await luaLspService.NotifyFileCloseAsync(file.Path);
        }

        public bool ConfirmCloseAll()
        {
            foreach (var file in files.Values) if (!ConfirmClose(file)) return false;
            return true;
        }

        public void DisposeLanguageService()
        {
            LuaDebugSession.StateChanged -= UpdateDebugStatus;
            completionWindow?.Close();
            luaLspService.DiagnosticsPublished -= LuaLspService_DiagnosticsPublished;
            luaLspService.Dispose();
        }

        // 텍스트 동기화만 담당
        private void LuaEditor_TextChanged(object sender, EventArgs e)
        {
            if (switchingFile || activeFile == null) return;
            ClearEditedBreakpoints();
            Dispatcher.InvokeAsync(UpdateFileStatus);
            if (!string.IsNullOrEmpty(targetPath))
            {
                IsDirty = true;
                EditorChange.Foreground = new SolidColorBrush(System.Windows.Media.Color.FromRgb(244, 190, 74));
                EditorChange.Text = "● Modified";
            }

            string currentText = LuaEditor.Text;
            int currentLine = LuaEditor.TextArea.Caret.Line - 1;
            int currentColumn = LuaEditor.TextArea.Caret.Column - 1;

            string changedPath = targetPath;
            _ = SyncDocumentAsync(changedPath, currentText, currentLine, currentColumn);
        }

        private async Task SyncDocumentAsync(string path, string text, int line, int column)
        {
            await lspReady;
            try { await luaLspService.SyncTextAsync(text, line, column, path); }
            catch (Exception) { }
        }

        // 자동완성 요청만 담당
        private async void TextArea_TextEntered(object sender, TextCompositionEventArgs e)
        {
            if (string.IsNullOrEmpty(e.Text)) return;
            char c = e.Text[0];
            if (char.IsLetterOrDigit(c) || c == '.' || c == ':')
            {
                int currentLine = LuaEditor.TextArea.Caret.Line - 1;
                int currentColumn = LuaEditor.TextArea.Caret.Column - 1;
                if (luaLspService != null)
                {
                    await luaLspService.RequestCompletionAsync(e.Text, currentLine, currentColumn);
                }
            }
        }

        private void luaEditor_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.F9)
            {
                ToggleBreakpoint(LuaEditor.TextArea.Caret.Line);
                e.Handled = true;
                return;
            }
            if (!LuaEditor.IsKeyboardFocusWithin) { e.Handled = true; }

            bool isCtrlPressed = (Keyboard.Modifiers & ModifierKeys.Control) != 0;
            bool isAltPressed = (Keyboard.Modifiers & ModifierKeys.Alt) != 0;

            if (isAltPressed && (e.SystemKey == Key.Up || e.SystemKey == Key.Down))
            {
                e.Handled = true;
                MoveSelectedLines(e.SystemKey == Key.Up ? -1 : 1);
                return;
            }

            if (isWaitingForFormatChord)
            {
                if (e.Key == Key.D)
                {
                    e.Handled = true;
                    isWaitingForFormatChord = false;
                    FormatLuaDocument();
                    return;
                }

                isWaitingForFormatChord = false;
            }

            if (isCtrlPressed)
            {
                if (e.Key == Key.K)
                {
                    e.Handled = true;
                    isWaitingForFormatChord = true;
                }
                else if (e.Key == Key.S)
                {
                    e.Handled = true;
                    SaveFile(activeFile);
                }
                else if (e.Key == Key.R)
                {
                    e.Handled = true;
                    if (SaveFile(activeFile)) PixelEngineNative.Reload();
                }
                else if (e.Key == Key.Space)
                {
                    int currentLine = LuaEditor.TextArea.Caret.Line - 1;
                    int currentColumn = LuaEditor.TextArea.Caret.Column - 1;
                    luaLspService.RequestCompletionAsync("", currentLine, currentColumn);
                }
            }
        }

        private void MoveSelectedLines(int direction)
        {
            TextDocument document = LuaEditor.Document;
            if (document == null || document.LineCount == 0) return;

            int selectionStart = LuaEditor.SelectionStart;
            int selectionLength = LuaEditor.SelectionLength;
            int selectionEnd = selectionStart + selectionLength;

            DocumentLine startLine = document.GetLineByOffset(selectionStart);
            DocumentLine endLine = document.GetLineByOffset(selectionEnd);
            if (selectionLength > 0 && selectionEnd == endLine.Offset && endLine.PreviousLine != null)
                endLine = endLine.PreviousLine;

            if (direction < 0)
            {
                DocumentLine previousLine = startLine.PreviousLine;
                if (previousLine == null) return;

                int previousDelimiterLength = previousLine.TotalLength - previousLine.Length;
                string previousContent = document.GetText(previousLine.Offset, previousLine.Length);
                string previousDelimiter = document.GetText(previousLine.EndOffset, previousDelimiterLength);
                int endLineTotalEnd = endLine.Offset + endLine.TotalLength;
                string selectedBlock = document.GetText(startLine.Offset, endLineTotalEnd - startLine.Offset);
                int selectedTrailingDelimiterLength = endLine.TotalLength - endLine.Length;

                string replacement = selectedTrailingDelimiterLength > 0
                    ? selectedBlock + previousContent + previousDelimiter
                    : selectedBlock + previousDelimiter + previousContent;

                int replaceLength = endLineTotalEnd - previousLine.Offset;
                int offsetDelta = -previousLine.TotalLength;
                using (document.RunUpdate())
                    document.Replace(previousLine.Offset, replaceLength, replacement);

                RestoreMovedSelection(selectionStart + offsetDelta, selectionLength);
            }
            else
            {
                DocumentLine nextLine = endLine.NextLine;
                if (nextLine == null) return;

                int selectedTrailingDelimiterLength = endLine.TotalLength - endLine.Length;
                string selectedDelimiter = document.GetText(endLine.EndOffset, selectedTrailingDelimiterLength);
                int endLineTotalEnd = endLine.Offset + endLine.TotalLength;
                int nextLineTotalEnd = nextLine.Offset + nextLine.TotalLength;
                string selectedBlock = document.GetText(startLine.Offset, endLineTotalEnd - startLine.Offset);
                string nextBlock = document.GetText(nextLine.Offset, nextLineTotalEnd - nextLine.Offset);
                int nextTrailingDelimiterLength = nextLine.TotalLength - nextLine.Length;

                string replacement = nextTrailingDelimiterLength > 0
                    ? nextBlock + selectedBlock
                    : document.GetText(nextLine.Offset, nextLine.Length) + selectedDelimiter +
                      selectedBlock.Substring(0, selectedBlock.Length - selectedTrailingDelimiterLength);

                int replaceLength = nextLineTotalEnd - startLine.Offset;
                int offsetDelta = nextTrailingDelimiterLength > 0
                    ? nextLine.TotalLength
                    : nextLine.Length + selectedTrailingDelimiterLength;
                using (document.RunUpdate())
                    document.Replace(startLine.Offset, replaceLength, replacement);

                RestoreMovedSelection(selectionStart + offsetDelta, selectionLength);
            }
        }

        private void RestoreMovedSelection(int selectionStart, int selectionLength)
        {
            if (selectionLength > 0)
            {
                LuaEditor.Select(selectionStart, selectionLength);
                LuaEditor.CaretOffset = selectionStart + selectionLength;
            }
            else
            {
                LuaEditor.CaretOffset = selectionStart;
            }

            LuaEditor.TextArea.Caret.BringCaretToView();
        }

        private void LuaEditor_PreviewMouseWheel(object sender, MouseWheelEventArgs e)
        {
            bool isCtrlPressed = (Keyboard.Modifiers & ModifierKeys.Control) != 0;
            if (!isCtrlPressed) return;

            double nextSize = LuaEditor.FontSize + (e.Delta > 0 ? 1.0 : -1.0);
            LuaEditor.FontSize = Math.Max(8.0, Math.Min(32.0, nextSize));
            e.Handled = true;
        }

        private void FormatLuaDocument()
        {
            if (LuaEditor.Document == null) return;

            LuaDocumentFormatter.Format(LuaEditor.Document);
        }

        private static class LuaDocumentFormatter
        {
            private static readonly Regex BlockStartRegex = new Regex(@"\b(function|then|do|repeat)\b", RegexOptions.Compiled);
            private static readonly Regex BlockEndRegex = new Regex(@"^\s*(end|until)\b", RegexOptions.Compiled);
            private static readonly Regex MiddleBlockRegex = new Regex(@"^\s*(else|elseif)\b", RegexOptions.Compiled);

            public static void Format(TextDocument document)
            {
                int indentLevel = 0;

                for (int lineNumber = 1; lineNumber <= document.LineCount; lineNumber++)
                {
                    DocumentLine line = document.GetLineByNumber(lineNumber);
                    string lineText = document.GetText(line);
                    string trimmedText = lineText.Trim();

                    if (trimmedText.Length == 0)
                    {
                        ReplaceIndent(document, line, string.Empty);
                        continue;
                    }

                    if (BlockEndRegex.IsMatch(trimmedText) || MiddleBlockRegex.IsMatch(trimmedText))
                    {
                        indentLevel = Math.Max(0, indentLevel - 1);
                    }

                    ReplaceIndent(document, line, new string(' ', indentLevel * 4));

                    if (BlockStartRegex.IsMatch(trimmedText) || MiddleBlockRegex.IsMatch(trimmedText))
                    {
                        indentLevel++;
                    }
                }
            }

            private static string GetIndent(string text)
            {
                int index = 0;
                while (index < text.Length && char.IsWhiteSpace(text[index]) && text[index] != '\r' && text[index] != '\n')
                {
                    index++;
                }
                return text.Substring(0, index);
            }

            private static void ReplaceIndent(TextDocument document, DocumentLine line, string indent)
            {
                string currentText = document.GetText(line);
                string currentIndent = GetIndent(currentText);
                document.Replace(line.Offset, currentIndent.Length, indent);
            }
        }

        private void SaveLuaFile(object sender, RoutedEventArgs e)
        {
            SaveFile(activeFile);
        }

        private void ReimportLuaFile(object sender, RoutedEventArgs e)
        {
            if (SaveFile(activeFile)) PixelEngineNative.Reload();
        }

        public TextArea GetLuaEditorTextArea()
        {
            return LuaEditor.TextArea;
        }

        private void Event_KeyUp(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetFunction("EventFunction", "KeyUpEvent");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }

        private void Event_KeyDown(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetFunction("EventFunction", "KeyDownEvent");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }

        private void CustomEvent(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetFunction("EventFunction", "CustomEvent");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }

        private void Create_Box2D(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetBlockByMarker("CreateCollider", "Box2DCollider");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }

        private void Create_Circle2D(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetBlockByMarker("CreateCollider", "Circle2DCollider");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }

        private void Create_RigidBody(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetBlockByMarker("CreateCollider", "RigidBody");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }

        private void Create_CollisionEnter(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetBlockByMarker("EventFunction", "OnCollisionEnter");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }

        private void Create_CollisionExit(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetBlockByMarker("EventFunction", "OnCollisionExit");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }

        private void Create_AnimationCallBack(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetBlockByMarker("EventFunction", "OnAnimationCallBack");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }

        private void Create_WaitForSeconds(object sender, RoutedEventArgs e)
        {
            string content = LuaFileManager.GetBlockByMarker("EventFunction", "WaitForSeconds");
            LuaEditor.Document.Insert(LuaEditor.CaretOffset, content + "\n\n");
        }
    }
}
