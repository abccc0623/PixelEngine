using ICSharpCode.AvalonEdit.CodeCompletion;
using Microsoft.VisualStudio.LanguageServer.Protocol;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace PixelTool
{
    internal class LuaCompletionData : ICompletionData
    {
        public LuaCompletionData(CompletionItem item)
        {
            Text = item.Label;
            kind = item.Kind;
            switch (item.Kind)
            {
                case CompletionItemKind.Text:emoji          = "[T]"; break;
                case CompletionItemKind.Interface:emoji     = "🌐";break;
                case CompletionItemKind.Class:emoji         = "📦";break;
                case CompletionItemKind.Function:emoji      = "🔷"; break;
                case CompletionItemKind.Method:emoji        = "🔷"; break;
                case CompletionItemKind.Variable: emoji     = "🔹"; break;
                case CompletionItemKind.Field: emoji        = "🔹"; break;
                default:
                    emoji = "";
                    break;
            }
        }
        public string Text { get; private set; } 
        public object Content 
        {
            get
            {
                var grid = new Grid();
                grid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto }); // 이모티콘 컬럼
                grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(5) }); // 간격 컬럼
                grid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto }); // 텍스트 컬럼

                // 1. 이모티콘 (아이콘 역할)
                var emojiBlock = new TextBlock
                {
                    Text = emoji,
                    Foreground = Brushes.Orange,
                    VerticalAlignment = VerticalAlignment.Center
                };
                Grid.SetColumn(emojiBlock, 0);

                // 2. 함수 이름
                var textBlock = new TextBlock
                {
                    Text = this.Text,
                    Foreground = Brushes.White, // 다크 테마용 글자색
                    VerticalAlignment = VerticalAlignment.Center
                };
                Grid.SetColumn(textBlock, 2);

                grid.Children.Add(emojiBlock);
                grid.Children.Add(textBlock);
                return grid;
            }
        }           
        public object Description { get; set; }  
        public double Priority => 0;

        public string emoji;
        public ImageSource Image => null;

        private CompletionItemKind kind;

        public void Complete(ICSharpCode.AvalonEdit.Editing.TextArea textArea, ICSharpCode.AvalonEdit.Document.ISegment completionSegment, EventArgs insertionRequestEventArgs)
        {
            string insertText = this.Text;

            // 2. 정규식으로 매개변수 예쁘게 갈아끼우기
            insertText = Regex.Replace(insertText, @"\bstring[0-9]*\b", "\"\"");
            insertText = Regex.Replace(insertText, @"\bnumber[0-9]*\b", "0");
            insertText = Regex.Replace(insertText, @"\bboolean[0-9]*\b", "false");

            // 3. (옵션) 변수일 경우 뒤에 콜론(:) 붙이기 
            // ※ 주의: kind 속성이 없다면 이 부분은 주석 처리하거나 네 코드에 맞게 수정해!
            // if (this.Kind == CompletionItemKind.Variable)
            // {
            //     insertText += ":";
            // }

            // 4. 🔥 [가장 중요] 덮어씌우기 전에 '시작 위치'를 안전하게 백업해 둠!
            int startOffset = completionSegment.Offset;

            // 5. 조립이 끝난 최종 텍스트를 단 *한 번만* 에디터에 덮어씌움
            textArea.Document.Replace(completionSegment, insertText);

            // 6. 10년 차의 미친 디테일: 커서 위치 맞추기
            int quoteIndex = insertText.IndexOf("\"\"");
            if (quoteIndex != -1)
            {
                // 쌍따옴표가 있으면 그 사이로 커서가 쏙! 들어감
                textArea.Caret.Offset = startOffset + quoteIndex + 1;
            }
            else if (insertText.EndsWith(")"))
            {
                // 괄호로 끝나면 괄호 안으로 커서 이동 (예: GetMousePosition_X(|))
                textArea.Caret.Offset = startOffset + insertText.Length - 1;
            }
            else
            {
                // 그 외의 경우는 그냥 입력된 단어의 맨 뒤로 이동
                textArea.Caret.Offset = startOffset + insertText.Length;
            }
        }
    }
}
