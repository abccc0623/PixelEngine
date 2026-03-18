using ICSharpCode.AvalonEdit.Highlighting;
using ICSharpCode.AvalonEdit.Highlighting.Xshd;
using System;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Xml;

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
                // 1. 텍스트를 "먼저" 넣습니다.
                LuaEditor.Text = "-- Welcome to Pixel Engine Lua Editor\nlocal a = 10\nfunction Test()\n    print('Hello World')\nend";

                // 2. 하이라이팅을 "나중에" 부릅니다.
                ApplyHardcodedHighlighting();

                // 3. 옵션 설정
                var options = LuaEditor.Options;
                options.ConvertTabsToSpaces = true;
            }
        }

        public void ApplyHardcodedHighlighting()
        {
            try
            {
                // [최후의 필살기] 파일 안 읽습니다. 그냥 코드에 박아버립니다.
                // XML 맨 앞에 빈 칸 하나라도 있으면 '토큰 에러' 납니다. 그래서 Trim()을 씁니다.
                string rawXml = @"
<SyntaxDefinition name=""LuaDark"" extensions="".lua"" xmlns=""http://icsharpcode.net/sharpdevelop/avalonedit/syntaxdefinition/2008"">
    <Color name=""Comment"" foreground=""#6A9955"" />
    <Color name=""String"" foreground=""#CE9178"" />
    <Color name=""Keywords"" foreground=""#569CD6"" fontWeight=""bold"" />
    <RuleSet>
        <Span color=""Comment""><Begin>--</Begin></Span>
        <Span color=""String""><Begin>""</Begin><End>""</End></Span>
        <Keywords color=""Keywords"">
            <Word>local</Word><Word>function</Word><Word>end</Word>
            <Word>if</Word><Word>then</Word><Word>else</Word><Word>return</Word>
        </Keywords>
    </RuleSet>
</SyntaxDefinition>".Trim();

                using (var sReader = new StringReader(rawXml))
                {
                    using (var xReader = XmlReader.Create(sReader))
                    {
                        LuaEditor.SyntaxHighlighting = HighlightingLoader.Load(xReader, HighlightingManager.Instance);
                    }
                }
            }
            catch (Exception ex)
            {
                // 만약 여기서도 에러 나면 에러 메시지 상자에 줄 번호(Line, Position)가 뜰 겁니다.
                MessageBox.Show("최후의 수단 실패: " + ex.Message);
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
}