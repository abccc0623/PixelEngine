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
        public LuaCompletionData(string text, string description = "")
        {
            this.Text = text;
            this.Description = description;
        }

        // 1. 왼쪽에 보여질 아이콘 이미지 (지금은 null이지만, 나중에 아이콘을 넣을 수 있습니다)
        public ImageSource Image { get { return null; } }

        // 2. 유저가 엔터를 쳤을 때 스크립트에 실제로 '삽입'될 문자열
        public string Text { get; private set; }

        // 3. 드롭다운 리스트에 '보여질' 내용 (보통 Text와 똑같이 둡니다)
        public object Content { get { return this.Text; } }

        // 4. 항목을 선택했을 때 옆에 뜨는 툴팁 설명창의 내용
        public object Description { get; private set; }

        // 5. 정렬 우선순위 (0이면 알파벳 순서대로 정렬됨)
        public double Priority { get { return 0; } }

        // 6. [핵심] 유저가 이 항목을 더블클릭하거나 엔터를 쳐서 '선택'했을 때 실행되는 동작
        public void Complete(TextArea textArea, ISegment completionSegment, EventArgs insertionRequestEventArgs)
        {
            // 에디터의 현재 커서 위치(completionSegment)에 우리가 가진 Text를 집어넣습니다!
            textArea.Document.Replace(completionSegment, this.Text);
        }
    }
}
