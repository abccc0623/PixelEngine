using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using ICSharpCode.AvalonEdit.Editing;
using ICSharpCode.AvalonEdit.Rendering;

namespace PixelTool
{
    internal sealed class LuaBreakpointMargin : AbstractMargin
    {
        private readonly Func<ISet<int>> getLines;
        private readonly Action<int> toggle;
        internal LuaBreakpointMargin(Func<ISet<int>> getLines, Action<int> toggle)
        {
            this.getLines = getLines; this.toggle = toggle;
            Cursor = Cursors.Hand;
            ToolTip = "클릭: 중단점 설정/해제 (F9). 저장·재시작 후 Play 모드에서 동작합니다.";
        }
        protected override Size MeasureOverride(Size availableSize) => new(20, 0);
        protected override void OnTextViewChanged(TextView oldTextView, TextView newTextView)
        {
            if (oldTextView != null) oldTextView.VisualLinesChanged -= LinesChanged;
            base.OnTextViewChanged(oldTextView, newTextView);
            if (newTextView != null) newTextView.VisualLinesChanged += LinesChanged;
        }
        private void LinesChanged(object? sender, EventArgs e) => InvalidateVisual();
        protected override void OnRender(DrawingContext drawingContext)
        {
            drawingContext.DrawRectangle(Brushes.Transparent, null, new Rect(RenderSize));
            if (TextView == null || !TextView.VisualLinesValid) return;
            var lines = getLines();
            foreach (var line in TextView.VisualLines)
                if (lines.Contains(line.FirstDocumentLine.LineNumber))
                {
                    double y = line.VisualTop - TextView.VerticalOffset + line.TextLines[0].Height / 2;
                    drawingContext.DrawEllipse(LuaDebugSession.Enabled ? Brushes.OrangeRed : Brushes.Transparent,
                        new Pen(Brushes.OrangeRed, 2), new Point(10, y), 5.5, 5.5);
                }
        }
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        {
            base.OnMouseLeftButtonDown(e);
            if (TextView == null || !TextView.VisualLinesValid) return;
            double y = e.GetPosition(this).Y + TextView.VerticalOffset;
            foreach (var line in TextView.VisualLines)
                if (y >= line.VisualTop && y < line.VisualTop + line.Height)
                { toggle(line.FirstDocumentLine.LineNumber); e.Handled = true; break; }
        }
    }
}
