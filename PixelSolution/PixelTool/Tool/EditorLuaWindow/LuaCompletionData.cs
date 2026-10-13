using ICSharpCode.AvalonEdit.CodeCompletion;
using Microsoft.VisualStudio.LanguageServer.Protocol;
using ICSharpCode.AvalonEdit.Snippets;
using Newtonsoft.Json.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace PixelTool
{
    internal class LuaCompletionData : ICompletionData
    {
        private readonly string snippetKind;
        private CompletionItem item;
        private readonly Func<CompletionItem,Task<CompletionItem>> resolver;
        private Task resolveTask;
        private readonly TextBlock description = new TextBlock { TextWrapping = TextWrapping.Wrap, MaxWidth = 500 };

        public LuaCompletionData(CompletionItem item, Func<CompletionItem,Task<CompletionItem>> resolver = null, bool engineApi = false)
        {
            this.item = item;
            this.resolver = resolver;
            ConfigureDescription();
            UpdateDescription();
            Text = item.Label;
            kind = item.Kind;
            switch (item.Kind)
            {
                case CompletionItemKind.Text:emoji          = "txt"; break;
                case CompletionItemKind.Interface:emoji     = "type";break;
                case CompletionItemKind.Class:emoji         = "type";break;
                case CompletionItemKind.Function:emoji      = "fn"; break;
                case CompletionItemKind.Method:emoji        = "fn"; break;
                case CompletionItemKind.Variable: emoji     = "var"; break;
                case CompletionItemKind.Field: emoji        = "field"; break;
                default:
                    emoji = "";
                    break;
            }
            if (engineApi && (item.Kind == CompletionItemKind.Field || item.Kind == CompletionItemKind.Variable ||
                item.Kind == CompletionItemKind.Class || item.Kind == CompletionItemKind.Interface || item.Kind == CompletionItemKind.Module))
                emoji = "API";
        }

        public LuaCompletionData(string label, string snippetKind)
        {
            Text = label;
            this.snippetKind = snippetKind;
            kind = CompletionItemKind.Snippet;
            emoji = "code";
            ConfigureDescription();
        }
        public string Text { get; private set; } 
        public object Content 
        {
            get
            {
                var grid = new Grid();
                grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(42) });
                grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(8) });
                grid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto }); // 텍스트 컬럼

                // 1. 이모티콘 (아이콘 역할)
                var emojiBlock = new TextBlock
                {
                    Text = emoji,
                    FontSize = 10,
                    HorizontalAlignment = HorizontalAlignment.Center,
                    VerticalAlignment = VerticalAlignment.Center
                };
                emojiBlock.SetResourceReference(TextBlock.ForegroundProperty,"PixelAccentBrush");
                var badge = new Border { Child = emojiBlock, CornerRadius = new CornerRadius(3), Padding = new Thickness(3,2,3,2), VerticalAlignment = VerticalAlignment.Center };
                badge.SetResourceReference(Border.BackgroundProperty,"PixelSurfaceBrush");
                Grid.SetColumn(badge, 0);

                // 2. 함수 이름
                var textBlock = new TextBlock
                {
                    Text = this.Text,
                    VerticalAlignment = VerticalAlignment.Center
                };
                Grid.SetColumn(textBlock, 2);

                grid.Children.Add(badge);
                grid.Children.Add(textBlock);
                return grid;
            }
        }           
        public object Description => description;
        public double Priority => 0;

        public string emoji;
        public ImageSource Image => null;

        private CompletionItemKind kind;

        private void ConfigureDescription()
        {
            description.FontFamily = new FontFamily("Consolas");
            description.FontSize = 12;
            description.SetResourceReference(TextBlock.ForegroundProperty,"PixelTextBrush");
            description.Loaded += (s,e) =>
            {
                DependencyObject parent = description;
                while (parent != null)
                {
                    if (parent is ToolTip tooltip)
                    {
                        tooltip.SetResourceReference(FrameworkElement.StyleProperty,"PixelCompletionToolTip");
                        break;
                    }
                    parent = VisualTreeHelper.GetParent(parent);
                }
            };
        }

        public async void Complete(ICSharpCode.AvalonEdit.Editing.TextArea textArea, ICSharpCode.AvalonEdit.Document.ISegment completionSegment, EventArgs insertionRequestEventArgs)
        {
            if (TryCompleteSnippet(textArea, completionSegment))
            {
                return;
            }

            var document = textArea.Document;
            var version = document.Version;
            int caret = textArea.Caret.Offset;
            await ResolveAsync();
            if (document != textArea.Document || version != document.Version || caret != textArea.Caret.Offset) return;

            try
            {

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

            var snippet = item?.InsertTextFormat == InsertTextFormat.Snippet
                ? BuildSnippet(insertText, item.Detail, textArea.Selection.GetText()) : null;
            var edits = new List<(int Offset, int Length, string Text)>();
            if (item?.AdditionalTextEdits != null)
            {
                foreach (var additional in item.AdditionalTextEdits)
                {
                    int from = document.GetOffset(additional.Range.Start.Line + 1, additional.Range.Start.Character + 1);
                    int to = document.GetOffset(additional.Range.End.Line + 1, additional.Range.End.Character + 1);
                    if (from < startOffset + length && to > startOffset) throw new InvalidOperationException("Overlapping completion edits.");
                    edits.Add((from,to - from,additional.NewText ?? ""));
                }
            }
            edits.Sort((a,b) => b.Offset.CompareTo(a.Offset));
            using (document.RunUpdate())
            {
                foreach (var additional in edits)
                {
                    document.Replace(additional.Offset,additional.Length,additional.Text);
                    if (additional.Offset <= startOffset) startOffset += additional.Text.Length - additional.Length;
                }
                if (snippet != null)
                {
                    textArea.Selection = ICSharpCode.AvalonEdit.Editing.Selection.Create(textArea,startOffset,startOffset + length);
                    snippet.Insert(textArea);
                }
                else
                {
                    document.Replace(startOffset,length,insertText);
                    textArea.Caret.Offset = startOffset + insertText.Length;
                }
            }
            }
            catch (Exception ex)
            {
                ConsoleWindow.LogMessage($"[LSP] 자동완성 삽입 실패: {ex.Message}",1);
            }
        }

        public Task ResolveAsync()
        {
            return resolveTask ??= ResolveCoreAsync();
        }

        private async Task ResolveCoreAsync()
        {
            if (resolver != null && item != null) item = await resolver(item);
            UpdateDescription();
        }

        private void UpdateDescription()
        {
            var documentation = item?.Documentation == null ? null : JToken.FromObject(item.Documentation);
            string content = documentation?.Type == JTokenType.String ? documentation.Value<string>() : documentation?["value"]?.Value<string>();
            description.Text = string.Join("\n", new[] { item?.Detail,content }.Where(value => !string.IsNullOrEmpty(value)));
        }

        private static Snippet BuildSnippet(string text, string detail, string selectedText)
        {
            var snippet = new Snippet();
            var fields = new Dictionary<int,SnippetReplaceableTextElement>();
            var quotedFields = new HashSet<int>();
            foreach (var part in LuaCompletionSnippet.Parse(text,detail,selectedText))
            {
                if (part.Index == 0)
                {
                    if (!string.IsNullOrEmpty(part.Text)) snippet.Elements.Add(new SnippetTextElement { Text = part.Text });
                    snippet.Elements.Add(new SnippetCaretElement());
                }
                else if (part.Index < 0)
                    snippet.Elements.Add(new SnippetTextElement { Text = part.Text });
                else if (fields.TryGetValue(part.Index,out var field))
                {
                    if (quotedFields.Contains(part.Index)) snippet.Elements.Add(new SnippetTextElement { Text = "\"" });
                    snippet.Elements.Add(new SnippetBoundElement { TargetElement = field });
                    if (quotedFields.Contains(part.Index)) snippet.Elements.Add(new SnippetTextElement { Text = "\"" });
                }
                else
                {
                    bool quoted = part.Text == "\"\"";
                    if (quoted) quotedFields.Add(part.Index);
                    if (quoted) snippet.Elements.Add(new SnippetTextElement { Text = "\"" });
                    field = new SnippetReplaceableTextElement { Text = quoted ? "" : part.Text };
                    fields.Add(part.Index,field);
                    snippet.Elements.Add(field);
                    if (quoted) snippet.Elements.Add(new SnippetTextElement { Text = "\"" });
                }
            }
            return snippet;
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
