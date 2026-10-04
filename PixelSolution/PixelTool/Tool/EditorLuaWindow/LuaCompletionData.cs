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
        private readonly string snippetKind;
        private readonly CompletionItem item;

        public LuaCompletionData(CompletionItem item)
        {
            this.item = item;
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

        public LuaCompletionData(string label, string snippetKind)
        {
            Text = label;
            this.snippetKind = snippetKind;
            kind = CompletionItemKind.Snippet;
            emoji = "[S]";
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
            if (TryCompleteSnippet(textArea, completionSegment))
            {
                return;
            }

            string insertText = item?.InsertText ?? this.Text;
            int startOffset = completionSegment.Offset;
            int length = completionSegment.Length;
            // LSP edits carry the replacement range as well as the insertion text.
            var edit = item?.TextEdit;
            var range = edit?.Range;
            if (range != null)
            {
                startOffset = textArea.Document.GetOffset(range.Start.Line + 1, range.Start.Character + 1);
                int endOffset = textArea.Document.GetOffset(range.End.Line + 1, range.End.Character + 1);
                length = endOffset - startOffset;
                insertText = edit.NewText ?? insertText;
            }

            insertText = PrepareInsertion(insertText, item?.Detail);
            textArea.Document.Replace(startOffset, length, insertText);

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

        internal static string PrepareInsertion(string text, string detail)
        {
            // LuaLS call snippets use argument names; signature details supply their types.
            text = Regex.Replace(text, @"\$\{\d+:([^{}]*)\}|\$\{\d+\}|\$\d+", match =>
                match.Groups[1].Success ? match.Groups[1].Value : string.Empty);
            int open = text.IndexOf('(');
            int close = text.LastIndexOf(')');
            if (open < 0 || close <= open) return text;
            string arguments = text.Substring(open + 1, close - open - 1);
            arguments = Regex.Replace(arguments, @"\b[A-Za-z_][A-Za-z_0-9]*\b", match =>
            {
                string type = match.Value;
                if (!string.IsNullOrEmpty(detail))
                {
                    var annotation = Regex.Match(detail, @"\b" + Regex.Escape(type) + @"\s*:\s*(string|number|integer|boolean)\b");
                    if (annotation.Success) type = annotation.Groups[1].Value;
                }
                switch (Regex.Replace(type, @"\d+$", string.Empty))
                {
                    case "string": return "\"\"";
                    case "number":
                    case "integer": return "0";
                    case "boolean": return "false";
                    default: return match.Value;
                }
            });
            return text.Substring(0, open + 1) + arguments + text.Substring(close);
        }

        private bool TryCompleteSnippet(ICSharpCode.AvalonEdit.Editing.TextArea textArea, ICSharpCode.AvalonEdit.Document.ISegment completionSegment)
        {
            string indent = GetCurrentIndent(textArea, completionSegment.Offset);
            string bodyIndent = indent + "    ";
            string snippet;
            int caretOffset;

            if (snippetKind == "for_ipairs")
            {
                snippet = $"for _, value in ipairs(items) do\n{bodyIndent}\n{indent}end";
                caretOffset = snippet.IndexOf("items", StringComparison.Ordinal);
            }
            else if (snippetKind == "for_pairs")
            {
                snippet = $"for key, value in pairs(items) do\n{bodyIndent}\n{indent}end";
                caretOffset = snippet.IndexOf("items", StringComparison.Ordinal);
            }
            else if (snippetKind == "for_numeric")
            {
                snippet = $"for i = 1, count do\n{bodyIndent}\n{indent}end";
                caretOffset = snippet.IndexOf("count", StringComparison.Ordinal);
            }
            else
            {
                return false;
            }

            int startOffset = completionSegment.Offset;
            textArea.Document.Replace(completionSegment, snippet);
            textArea.Caret.Offset = startOffset + (caretOffset >= 0 ? caretOffset : snippet.Length);
            return true;
        }

        private static string GetCurrentIndent(ICSharpCode.AvalonEdit.Editing.TextArea textArea, int offset)
        {
            var line = textArea.Document.GetLineByOffset(offset);
            string lineText = textArea.Document.GetText(line.Offset, line.Length);
            int index = 0;
            while (index < lineText.Length && char.IsWhiteSpace(lineText[index]))
            {
                index++;
            }
            return lineText.Substring(0, index);
        }
    }
}
