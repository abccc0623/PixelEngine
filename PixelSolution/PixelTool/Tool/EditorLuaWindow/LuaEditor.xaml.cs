using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.CodeCompletion;
using ICSharpCode.AvalonEdit.Editing;
using ICSharpCode.AvalonEdit.Rendering; // 핵심: 직접 색칠하는 도구
using PixelTool;
using System;
using System.IO;
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
        }

        private async void InitializeEditor()
        {
            luaLspService = new LuaLspService();

            luaLspService.StartServer();
            await Task.Delay(500);
            await luaLspService.SendInitializeRequest();
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
                if(IsDirty == true)
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
                // 이미 자동완성 창이 떠있으면 무시
                if (completionWindow != null) return;

                // 1. 현재 커서 위치와 에디터의 "최신 전체 텍스트" 가져오기
                var caret = LuaEditor.TextArea.Caret;
                int line = caret.Line;
                int col = caret.Column;
                string currentText = LuaEditor.Text;

                // 2. Task.Run으로 비동기 순차 처리 (UI 프리징 방지 + 서버 크래시 방지)
                Task.Run(async () => {
                    try
                    {
                        // [생존 수칙 1] 방금 찍은 점(.)이 포함된 최신 텍스트를 서버에 먼저 동기화합니다.
                        // 이거 안 하면 서버가 "어? 점이 없는데 왜 여기서 자동완성을 찾지?" 하고 뒤집니다.
                        await luaLspService.SendDidChangeNotification(targetPath, currentText);

                        // [생존 수칙 2] 서버가 텍스트를 읽고 소화할 시간을 아주 살짝(50ms) 줍니다.
                        await Task.Delay(50);

                        // [생존 수칙 3] 이제 안전하게 자동완성 목록을 요청합니다.
                        await luaLspService.SendCompletionRequest(targetPath, line, col);
                    }
                    catch (Exception ex)
                    {
                        System.Diagnostics.Debug.WriteLine($"!!! 자동완성 요청 중 에러: {ex.Message}");
                    }
                });
            }
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
