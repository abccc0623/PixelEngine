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

        public CompletionWindow completionWindow;
        private CancellationTokenSource _debounceTokenSource;
        private Dictionary<string, string> variableTypes = new Dictionary<string, string>();
        private LuaLspService luaLspService;
        private bool isWaitingForFormatChord = false;

        public LuaEditorWindow()
        {
            InitializeComponent();
            luaLspService = new LuaLspService();
            luaLspService.Initialize();

            ConfigureEditorOptions();
            LuaEditor.TextChanged += LuaEditor_TextChanged;
            LuaEditor.TextArea.TextEntered += TextArea_TextEntered;
            LuaEditor.PreviewMouseWheel += LuaEditor_PreviewMouseWheel;
            ApplyLuaSyntaxHighlighting();
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

        public async void OpenFile(string path)
        {
            if (System.IO.File.Exists(path))
            {
                if (EditorChange.Foreground == Brushes.Red)
                {
                    var result = MessageBox.Show("파일을 변경전 저장이 필요합니다.\n", "저장", MessageBoxButton.YesNo, MessageBoxImage.Warning);
                    if (result == MessageBoxResult.Yes)
                    {
                        LuaEditor.Save(targetPath);
                    }
                }

                LuaEditor.Load(path);
                targetPath = path;
                if (luaLspService != null)
                {
                    await luaLspService.NotifyFileOpenAsync(path, LuaEditor.Text);
                }
                LuaEditor.Document.UndoStack.MarkAsOriginalFile();
                IsDirty = false;
                EditorChange.Foreground = Brushes.Green;
            }
        }

        // 텍스트 동기화만 담당
        private void LuaEditor_TextChanged(object sender, EventArgs e)
        {
            string currentText = LuaEditor.Text;
            int currentLine = LuaEditor.TextArea.Caret.Line - 1;
            int currentColumn = LuaEditor.TextArea.Caret.Column - 1;

            Task.Run(async () =>
            {
                try
                {
                    await luaLspService.SyncTextAsync(currentText, currentLine, currentColumn);
                }
                catch (Exception) { }
            });
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
            if (!LuaEditor.IsKeyboardFocusWithin) { e.Handled = true; }

            bool isCtrlPressed = (Keyboard.Modifiers & ModifierKeys.Control) != 0;

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
                    LuaEditor.Save(targetPath);
                    EditorChange.Foreground = Brushes.Green;
                }
                else if (e.Key == Key.R)
                {
                    e.Handled = true;
                    LuaEditor.Save(targetPath);
                    PixelEngineNative.Reload();
                }
                else if (e.Key == Key.Space)
                {
                    int currentLine = LuaEditor.TextArea.Caret.Line - 1;
                    int currentColumn = LuaEditor.TextArea.Caret.Column - 1;
                    luaLspService.RequestCompletionAsync("", currentLine, currentColumn);
                }
            }
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
            LuaEditor.Save(targetPath);
        }

        private void ReimportLuaFile(object sender, RoutedEventArgs e)
        {
            PixelEngineNative.Reload();
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
    }
}
