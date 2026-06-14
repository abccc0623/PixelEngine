using ICSharpCode.AvalonEdit.Document;
using ICSharpCode.AvalonEdit.Rendering;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Media;

namespace PixelTool
{
    internal sealed class LuaDiagnosticSegment : ISegment
    {
        public LuaDiagnosticSegment(int offset, int length)
        {
            Offset = offset;
            Length = length;
        }

        public int Offset { get; }
        public int Length { get; }
        public int EndOffset => Offset + Length;
    }

    internal sealed class LuaDiagnosticRenderer : IBackgroundRenderer
    {
        private readonly TextView _textView;
        private readonly Pen _errorPen;
        private IReadOnlyList<ISegment> _errorSegments = Array.Empty<ISegment>();

        public LuaDiagnosticRenderer(TextView textView)
        {
            _textView = textView;
            _errorPen = new Pen(new SolidColorBrush(Color.FromRgb(244, 71, 71)), 1.2);
            _errorPen.Freeze();
        }

        public KnownLayer Layer => KnownLayer.Selection;

        public void SetErrors(IReadOnlyList<ISegment> errorSegments)
        {
            _errorSegments = errorSegments ?? Array.Empty<ISegment>();
            _textView.InvalidateLayer(Layer);
        }

        public void Draw(TextView textView, DrawingContext drawingContext)
        {
            if (!_textView.VisualLinesValid || _errorSegments.Count == 0) return;

            foreach (ISegment segment in _errorSegments)
            {
                foreach (Rect rect in BackgroundGeometryBuilder.GetRectsForSegment(textView, segment))
                {
                    DrawSquiggle(drawingContext, rect);
                }
            }
        }

        private void DrawSquiggle(DrawingContext drawingContext, Rect rect)
        {
            const double waveWidth = 4.0;
            const double waveHeight = 2.0;
            double startX = rect.Left;
            double endX = Math.Max(rect.Right, startX + waveWidth);
            double y = rect.Bottom - 1.0;

            var geometry = new StreamGeometry();
            using (StreamGeometryContext context = geometry.Open())
            {
                context.BeginFigure(new Point(startX, y), false, false);
                bool rise = true;
                for (double x = startX + waveWidth / 2.0; x < endX; x += waveWidth / 2.0)
                {
                    context.LineTo(new Point(x, y + (rise ? -waveHeight : 0.0)), true, false);
                    rise = !rise;
                }
                context.LineTo(new Point(endX, y + (rise ? -waveHeight : 0.0)), true, false);
            }

            geometry.Freeze();
            drawingContext.DrawGeometry(null, _errorPen, geometry);
        }
    }
}
