using System;
using System.IO;
using System.Text.RegularExpressions; // 정규식 사용
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;           // 색상 사용
using ICSharpCode.AvalonEdit.Rendering; // 핵심: 직접 색칠하는 도구

namespace PixelTool
{
    public partial class LuaEditorWindow : UserControl
    {
        public static LuaEditorWindow Instance { get; private set; }

        public LuaEditorWindow()
        {
            InitializeComponent();
            Instance = this;

            if (LuaEditor != null)
            {
                // 1. 기존 XML 하이라이팅을 완전히 끕니다. (충돌 방지)
                LuaEditor.SyntaxHighlighting = null;

                // 2. [핵심] 우리가 C#으로 만든 "직접 색칠하는 페인터"를 에디터에 장착합니다.
                LuaEditor.TextArea.TextView.LineTransformers.Add(new LuaDarkColorizer());

                var options = LuaEditor.Options;
                options.ConvertTabsToSpaces = true;

                // 테스트용 텍스트
                LuaEditor.Text = "-- Welcome to Pixel Engine\nlocal a = 10\nprint(\"Hello World\")\nfunction Test()\n    Engine.Init()\nend";
            }
        }

        public void OpenFile(string path)
        {
            if (System.IO.File.Exists(path))
            {
                LuaEditor.Load(path);
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