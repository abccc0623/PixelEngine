using ICSharpCode.AvalonEdit.CodeCompletion;
using ICSharpCode.AvalonEdit.Document;
using ICSharpCode.AvalonEdit.Editing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace PixelTool
{
    internal class LuaCompletionData : ICompletionData
    {
        public LuaCompletionData(string text, string detail)
        {
            this.Text = text;
            this.Description = detail; // 마우스 올리면 나오는 설명
        }
        public ImageSource Image => null;
        public string Text { get; private set; }
        public object Content => Text;
        public object Description { get; private set; }
        public double Priority => 0;
        public void Complete(ICSharpCode.AvalonEdit.Editing.TextArea textArea,
            ICSharpCode.AvalonEdit.Document.ISegment completionSegment, EventArgs insertionRequestEventArgs)
        {
            textArea.Document.Replace(completionSegment, this.Text);
        }
    }
}
