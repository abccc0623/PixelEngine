using ICSharpCode.AvalonEdit;
using ICSharpCode.AvalonEdit.Rendering; // 핵심: 직접 색칠하는 도구
using System;
using System.IO;
using System.Text.RegularExpressions; // 정규식 사용
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;           // 색상 사용

namespace PixelTool
{
    public partial class LuaEditorWindow : UserControl
    {
        private string targetPath = "";
        public static LuaEditorWindow Instance { get; private set; }

        public LuaEditorWindow()
        {
            InitializeComponent();
            Instance = this;

            this.Loaded += (s, e) =>
            {
                // 1. 현재 UserControl을 담고 있는 진짜 부모 Window를 찾음
                Window parentWindow = Window.GetWindow(this);

                if (parentWindow != null)
                {
                    // 2. 윈도우가 포커스를 얻었을 때 (엔진 창 클릭 시) -> 입력 가능
                    parentWindow.Activated += (sender, args) => LuaEditor.IsReadOnly = false;
                    // 3. 윈도우가 포커스를 잃었을 때 (바탕화면이나 다른 창 클릭 시) -> 입력 차단
                    parentWindow.Deactivated += (sender, args) => LuaEditor.IsReadOnly = true;
                }
            };

            if (LuaEditor != null)
            {
                // 1. 기존 XML 하이라이팅을 완전히 끕니다. (충돌 방지)
                LuaEditor.SyntaxHighlighting = null;

                // 2. [핵심] 우리가 C#으로 만든 "직접 색칠하는 페인터"를 에디터에 장착합니다.
                LuaEditor.TextArea.TextView.LineTransformers.Add(new LuaDarkColorizer());

                var options = LuaEditor.Options;
                options.ConvertTabsToSpaces = true;

                // 테스트용 텍스트
                LuaEditor.Text  = "-- Welcome to Pixel Engine -- \n";
                LuaEditor.Text += "Click 'main.lua' to add game logic.";
            }
        }

        public void OpenFile(string path)
        {
            if (System.IO.File.Exists(path))
            {
                LuaEditor.Load(path);
                targetPath = path;

                LuaEditor.Document.UndoStack.MarkAsOriginalFile();
                UpdateTitle();
            }
        }

        private void UpdateTitle()
        {
            // UndoStack이 원본 위치에 있는지 확인
            bool isDirty = !LuaEditor.Document.UndoStack.IsOriginalFile;

            string title = "Lua Editor";
            if (isDirty) title += " *"; // 변경됨 표시

            // Window 타이틀이나 탭 이름 업데이트
            //this.Title = title;
        }

        public void SaveFile()
        {

        }

        private void SaveCommand_CanExecute(object sender, System.Windows.Input.CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = this.IsKeyboardFocusWithin;
        }

     
        private void SaveCommand_Executed(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            if(!string.IsNullOrWhiteSpace(targetPath))
            {
                LuaEditor.Save(targetPath);
                PixelEngineNative.Reload();
            }
        }

        private void luaEditor_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            // 에디터에 포커스가 없으면 모든 키 입력 이벤트를 처리된 것으로 간주(Handled)하여 무시
            if (!LuaEditor.IsKeyboardFocusWithin)
            {
                e.Handled = true;
            }
        }
    }

    // =========================================================================
    // [완전히 새로운 방식] XML 없이 C#으로 직접 글자색을 칠하는 클래스입니다.
    // =========================================================================
    public class LuaDarkColorizer : DocumentColorizingTransformer
    {
        protected override void ColorizeLine(ICSharpCode.AvalonEdit.Document.DocumentLine line)
        {
            int lineStartOffset = line.Offset;
            string text = CurrentContext.Document.GetText(line);

            // 1. 키워드 (파란색 #569CD6)
            string[] keywords = { "local", "function", "end", "if", "then", "else", "return", "print", "Engine", "Asset" };
            foreach (string keyword in keywords)
            {
                // 단어 단위로 정확히 찾기 위해 정규식(\b) 사용
                foreach (Match m in Regex.Matches(text, @"\b" + keyword + @"\b"))
                {
                    ChangeLinePart(
                        lineStartOffset + m.Index,
                        lineStartOffset + m.Index + m.Length,
                        (VisualLineElement element) =>
                        {
                            element.TextRunProperties.SetForegroundBrush(new SolidColorBrush(Color.FromRgb(86, 156, 214)));
                            element.TextRunProperties.SetTypeface(new Typeface(element.TextRunProperties.Typeface.FontFamily, FontStyles.Normal, FontWeights.Bold, FontStretches.Normal));
                        });
                }
            }

            // 2. 문자열 (" " 또는 ' ') (주황색 #CE9178)
            foreach (Match m in Regex.Matches(text, "\".*?\"|'.*?'"))
            {
                ChangeLinePart(
                    lineStartOffset + m.Index,
                    lineStartOffset + m.Index + m.Length,
                    (VisualLineElement element) =>
                    {
                        element.TextRunProperties.SetForegroundBrush(new SolidColorBrush(Color.FromRgb(206, 145, 120)));
                    });
            }

            // 3. 주석 (--) (초록색 #6A9955)
            int commentIndex = text.IndexOf("--");
            if (commentIndex >= 0)
            {
                ChangeLinePart(
                    lineStartOffset + commentIndex,
                    lineStartOffset + text.Length,
                    (VisualLineElement element) =>
                    {
                        element.TextRunProperties.SetForegroundBrush(new SolidColorBrush(Color.FromRgb(106, 153, 85)));
                    });
            }
        }
    }
}