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
                    LuaEditor.Save(targetPath);
                    EditorChange.Foreground = Brushes.Green;
                }
                else if (e.Key == Key.R)
                {
                    e.Handled = true;
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


        private void CreateEventFunction(object sender, string fileName)
        {
            if (sender is MenuItem clickedMenu && clickedMenu.Tag != null)
            {
                // 1. Tag에서 찾고자 하는 함수 이름(예: Event_KeyDown)을 가져옵니다.
                string targetFunction = clickedMenu.Tag.ToString();

                // 2. 모든 함수가 들어있는 단일 루아 파일의 리소스 경로입니다. (실제 경로에 맞게 수정하세요)
                string resourceName = $"PixelTool.LuaCode.{fileName}.lua";

                using (var stream = System.Reflection.Assembly.GetExecutingAssembly().GetManifestResourceStream(resourceName))
                {
                    if (stream == null)
                    {
                        MessageBox.Show("공통 루아 리소스 파일을 찾을 수 없습니다.");
                        return;
                    }

                    using (var reader = new System.IO.StreamReader(stream))
                    {
                        // 3. 파일의 전체 내용을 읽어옵니다.
                        string fullCode = reader.ReadToEnd();

                        // 4. 전체 코드에서 원하는 함수 부분만 추출합니다.
                        string extractedCode = ExtractFunctionBlock(fullCode, targetFunction);

                        if (!string.IsNullOrEmpty(extractedCode))
                        {
                            // 5. 추출된 코드를 에디터에 삽입하고, 줄 바꿈을 넣어 깔끔하게 정리합니다.
                            LuaEditor.Document.Insert(LuaEditor.CaretOffset, extractedCode + "\n\n");
                        }
                        else
                        {
                            MessageBox.Show($"{targetFunction} 함수를 파일에서 찾을 수 없습니다.");
                        }
                    }
                }
            }
        }

        private string ExtractFunctionBlock(string fullCode, string functionName)
        {
            // 찾을 시작 키워드 (예: "function self:Event_KeyDown")
            string startKeyword = $"function self:{functionName}";
            int startIndex = fullCode.IndexOf(startKeyword);

            // 해당 함수가 없으면 빈 문자열 반환
            if (startIndex == -1) return "";

            // 현재 함수 내용이 끝나는 지점을 찾기 위해, '다음 함수'가 시작되는 위치를 찾습니다.
            int nextFunctionIndex = fullCode.IndexOf("function self:", startIndex + startKeyword.Length);

            if (nextFunctionIndex != -1)
            {
                // 다음 함수가 존재한다면, 현재 함수 시작점부터 다음 함수 시작점 전까지 잘라냅니다.
                return fullCode.Substring(startIndex, nextFunctionIndex - startIndex).TrimEnd();
            }
            else
            {
                // 다음 함수가 없다면 (즉, 파일의 마지막 함수라면) 끝까지 잘라냅니다.
                return fullCode.Substring(startIndex).TrimEnd();
            }
        }

        private void Event_KeyUp(object sender, RoutedEventArgs e)
        {
            CreateEventFunction(sender, "Event_Key");
        }
        private void Event_KeyDown(object sender, RoutedEventArgs e)
        {
            CreateEventFunction(sender, "Event_Key");
        }

        private void CustomEvent(object sender, RoutedEventArgs e)
        {
            CreateEventFunction(sender, "Event_Key");
        }
    }
}
