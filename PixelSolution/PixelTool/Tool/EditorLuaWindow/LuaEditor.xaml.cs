using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.CodeCompletion;
using ICSharpCode.AvalonEdit.Editing;
using ICSharpCode.AvalonEdit.Highlighting;
using ICSharpCode.AvalonEdit.Highlighting.Xshd;
using ICSharpCode.AvalonEdit.Rendering; // 핵심: 직접 색칠하는 도구
using Microsoft.VisualStudio.LanguageServer.Protocol;
using PixelTool;
using System;
using System.IO;
using System.Reflection;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.RegularExpressions; // 정규식 사용
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;           // 색상 사용

namespace PixelTool
{
    public partial class LuaEditorWindow : UserControl
    {
        private string targetPath = "";
        private bool IsDirty = false;

        public CompletionWindow completionWindow;
        private CancellationTokenSource _debounceTokenSource;


        private System.Windows.Threading.DispatcherTimer _typeTimer;
        private Dictionary<string, string> variableTypes = new Dictionary<string, string>();

        private LuaLspService luaLspService;

        public LuaEditorWindow()
        {
            InitializeComponent();
            luaLspService = new LuaLspService();
            luaLspService.Initialize();

            LuaEditor.TextChanged += LuaEditor_TextChanged;
            LuaEditor.TextArea.TextEntered += TextArea_TextEntered;
            //InitializeEditor();
            ApplyLuaSyntaxHighlighting();
        }

        private void ApplyLuaSyntaxHighlighting()
        {
            try
            {
                // "프로젝트이름.폴더이름.파일명.xshd" 형식인지 꼭 확인하세요!
                var resourceName = "PixelTool.Tool.EditorLuaWindow.Lua.xshd";
                using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
                {
                    if (stream == null) return; // 파일을 못 찾으면 그냥 리턴 (에러 방지)
            
                    using (var reader = new System.Xml.XmlTextReader(stream))
                    {
                        LuaEditor.SyntaxHighlighting = ICSharpCode.AvalonEdit.Highlighting.Xshd.HighlightingLoader.Load(reader, HighlightingManager.Instance);
                    }
                }
            }
            catch (Exception ex)
            {
                // 리소스 로드 실패 시 디버깅용
                System.Diagnostics.Debug.WriteLine("Highlighting Load Failed: " + ex.Message);
                ConsoleWindow.LogMessage("Highlighting Load Failed: " + ex.Message, 2);
            }
        }


        public async void OpenFile(string path)
        {
            if (System.IO.File.Exists(path))
            {
                if(EditorChange.Foreground == Brushes.Red)
                {
                    var result = MessageBox.Show("파일을 변경전 저장이 필요합니다.\n","저장",MessageBoxButton.YesNo,MessageBoxImage.Warning);
                    if (result == MessageBoxResult.Yes)
                    {
                        LuaEditor.Save(targetPath);
                    }
                }

                LuaEditor.Load(path);
                targetPath = path;
                string content = LuaEditor.Text;
                if (luaLspService != null)
                {
                    await luaLspService.NotifyFileOpenAsync(path, LuaEditor.Text);
                }
                LuaEditor.Document.UndoStack.MarkAsOriginalFile();
                IsDirty = false;
                EditorChange.Foreground = Brushes.Green;
            }
        }

        private async void TextArea_TextEntered(object sender, TextCompositionEventArgs e)
        {
            if (string.IsNullOrEmpty(e.Text)) return;
            char c = e.Text[0];
            if (char.IsLetterOrDigit(c) || c == '.' || c == ':')
            {
                if(completionWindow != null){completionWindow.Close();}
                int currentLine = LuaEditor.TextArea.Caret.Line - 1;
                int currentColumn = LuaEditor.TextArea.Caret.Column -1;
                if (luaLspService != null)
                {
                    await luaLspService.NotifyDidChangeAsync(LuaEditor.Text, currentLine, currentColumn, e.Text);
                }
            }
        }

        private void luaEditor_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            // 에디터에 포커스가 없으면 모든 키 입력 이벤트를 처리된 것으로 간주(Handled)하여 무시
            if (!LuaEditor.IsKeyboardFocusWithin){e.Handled = true;}
            
            bool isCtrlPressed = (Keyboard.Modifiers & ModifierKeys.Control) != 0;
            
            if (isCtrlPressed)
            {
                if(e.Key == Key.S)
                {
                    e.Handled = true;
                    //GeneratedLuaModuleType();
                    LuaEditor.Save(targetPath);
                    EditorChange.Foreground = Brushes.Green;
                }
                else if (e.Key == Key.R)
                {
                    e.Handled = true;
                    //GeneratedLuaModuleType();
                    LuaEditor.Save(targetPath);
                    PixelEngineNative.Reload();
                }
                else if (e.Key == Key.Space && (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
                {
                    int currentLine = LuaEditor.TextArea.Caret.Line - 1;
                    int currentColumn = LuaEditor.TextArea.Caret.Column - 1;
                    luaLspService.NotifyDidChangeAsync(LuaEditor.Text, currentLine, currentColumn,"");
                }
            }
        }

        private void LuaEditor_TextChanged(object sender, EventArgs e)
        {
            if (completionWindow != null) return;
            
            _debounceTokenSource?.Cancel();
            _debounceTokenSource = new CancellationTokenSource();
            var token = _debounceTokenSource.Token;
            
            string currentText = LuaEditor.Text;
            int currentLine = LuaEditor.TextArea.Caret.Line - 1;
            int currentColumn = LuaEditor.TextArea.Caret.Column - 1;
            Task.Run(async () =>
            {
                try
                {
                    if (!token.IsCancellationRequested)
                    {
                        await luaLspService.NotifyDidChangeAsync(currentText, currentLine, currentColumn,"");
                    }
                }
                catch (TaskCanceledException) { /* 타이머 취소됨 (정상적인 동작) */ }
            });
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

        //GENERATED
        void GeneratedLuaModuleType()
        {
            string ext = Path.GetExtension(targetPath);
            if (ext != ".pxm") return;

            string originalCode = LuaEditor.Text;


            // 패턴 설명:
            // (---@type\s+\w+\s+)? : 이미 존재하는 주석 (있을 수도 있고 없을 수도 있음)
            // (self\.([a-zA-Z0-9_]+)\s*=\s*.*:AddModule\("([a-zA-Z0-9_]+)"\)) : AddModule 할당 코드
            string pattern = @"(---@type\s+\w+\s+)?(self\.([a-zA-Z0-9_]+)\s*=\s*.*:AddModule\(""([a-zA-Z0-9_]+)""\))";

            LuaEditor.Text = Regex.Replace(originalCode, pattern, m =>
            {
                string currentCodeLine = m.Groups[2].Value; // 실제 코드 (self.xxx = ...)
                string typeName = m.Groups[4].Value;       // 모듈 타입 (Movement 등)

                // 기존 주석 여부 상관없이 "최신 타입 주석 + 코드" 형태로 반환
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
    }
}
