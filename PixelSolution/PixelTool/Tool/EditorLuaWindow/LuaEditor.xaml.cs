using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.CodeCompletion;
using ICSharpCode.AvalonEdit.Editing;
using ICSharpCode.AvalonEdit.Highlighting;
using ICSharpCode.AvalonEdit.Highlighting.Xshd;
using ICSharpCode.AvalonEdit.Rendering; // 핵심: 직접 색칠하는 도구
using PixelTool;
using System;
using System.IO;
using System.Reflection;
using System.Text.RegularExpressions; // 정규식 사용
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

        private CompletionWindow completionWindow;
        private System.Windows.Threading.DispatcherTimer _typeTimer;
        private Dictionary<string, string> variableTypes = new Dictionary<string, string>();

        private LuaLspService luaLspService;

        public LuaEditorWindow()
        {
            InitializeComponent();

            _typeTimer = new System.Windows.Threading.DispatcherTimer();
            _typeTimer.Interval = TimeSpan.FromMilliseconds(300); // 0.3초 대기
            _typeTimer.Tick += OnTypeTimerTick;
            _typeTimer.Start();
            LuaEditor.TextChanged += LuaEditor_TextChanged;
            LuaEditor.TextArea.TextEntered += TextArea_TextEntered;
            InitializeEditor();
            ApplyLuaSyntaxHighlighting();
        }

        private async void InitializeEditor()
        {
            luaLspService = new LuaLspService();

            luaLspService.StartServer();
            await Task.Delay(500);
            await luaLspService.SendInitializeRequest();
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

        private async void OnTypeTimerTick(object sender, EventArgs e)
        {
            _typeTimer.Stop();
            if (!string.IsNullOrEmpty(targetPath))
            {
                await luaLspService.SendDidChangeNotification(targetPath, LuaEditor.Text);
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
                    await luaLspService.SendDidOpenNotification(path, content);
                }
                LuaEditor.Document.UndoStack.MarkAsOriginalFile();
                IsDirty = false;
                EditorChange.Foreground = Brushes.Green;
            }
        }

        private void TextArea_TextEntered(object sender, TextCompositionEventArgs e)
        {
            // . 이나 : 을 입력했을 때 자동완성 요청
            if (e.Text == "." || e.Text == ":")
            {
                if (completionWindow != null) return;

                // [데이터 복사] UI 스레드에서 안전하게 값만 추출합니다.
                var caret = LuaEditor.TextArea.Caret;
                int line = caret.Line;
                int col = caret.Column;
                string currentText = LuaEditor.Text;
                string currentPath = this.targetPath; // 경로도 미리 복사

                // 비동기 작업 시작
                Task.Run(async () => {
                    try
                    {
                        // [동기화] 서버에 현재 텍스트 전달
                        await luaLspService.SendDidChangeNotification(currentPath, currentText);

                        // [지연] 서버가 인덱싱할 시간을 아주 살짝 줍니다 (LSP 사양에 따라 조절 가능)
                        await Task.Delay(50);

                        // [요청] 자동완성 목록 요청
                        await luaLspService.SendCompletionRequest(currentPath, line, col);
                    }
                    catch (Exception ex)
                    {
                        // 로그 출력 시에도 Dispatcher를 활용해 UI 스레드 충돌 방지
                        App.Current.Dispatcher.BeginInvoke(new Action(() => {
                            ConsoleWindow.LogMessage($"LSP 자동완성 실패: {ex.Message}", 2);
                        }));
                    }
                });
            }
            EditorChange.Foreground = Brushes.Red;
        }

        private void luaEditor_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            // 에디터에 포커스가 없으면 모든 키 입력 이벤트를 처리된 것으로 간주(Handled)하여 무시
            if (!LuaEditor.IsKeyboardFocusWithin){e.Handled = true;}
            
            bool isCtrlPressed = (Keyboard.Modifiers & ModifierKeys.Control) != 0;
            
            if (isCtrlPressed)
            {
                switch (e.Key)
                {
                    case Key.S: // 저장만
                        e.Handled = true;
                        LuaEditor.Save(targetPath);
                        EditorChange.Foreground = Brushes.Green;
                        break;
                    case Key.R: // 저장 + 리로드 (ReImport)
                        e.Handled = true;
                        LuaEditor.Save(targetPath);
                        PixelEngineNative.Reload();
                        break;
                }
            }
        }

        private void LuaEditor_TextChanged(object sender, EventArgs e)
        {
            _typeTimer.Stop();
            _typeTimer.Start();
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
    }
}
