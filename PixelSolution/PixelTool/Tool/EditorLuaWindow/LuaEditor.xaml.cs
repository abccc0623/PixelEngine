using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.CodeCompletion;
using ICSharpCode.AvalonEdit.Editing;
using ICSharpCode.AvalonEdit.Highlighting;
using ICSharpCode.AvalonEdit.Highlighting.Xshd;
using ICSharpCode.AvalonEdit.Rendering;
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

        public LuaEditorWindow()
        {
            InitializeComponent();
            luaLspService = new LuaLspService();
            luaLspService.Initialize();

            LuaEditor.TextChanged += LuaEditor_TextChanged;
            LuaEditor.TextArea.TextEntered += TextArea_TextEntered;
            ApplyLuaSyntaxHighlighting();
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

            if (isCtrlPressed)
            {
                if (e.Key == Key.S)
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

        void GeneratedLuaModuleType()
        {
            string ext = Path.GetExtension(targetPath);
            if (ext != ".pxm") return;

            string originalCode = LuaEditor.Text;
            string pattern = @"(---@type\s+\w+\s+)?(self\.([a-zA-Z0-9_]+)\s*=\s*.*:AddModule\(""([a-zA-Z0-9_]+)""\))";

            LuaEditor.Text = Regex.Replace(originalCode, pattern, m =>
            {
                string currentCodeLine = m.Groups[2].Value;
                string typeName = m.Groups[4].Value;
                return $"---@type {typeName}\n    {currentCodeLine}";
            }, RegexOptions.Multiline);
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