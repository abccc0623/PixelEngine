using System.IO;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace PixelTool;

// A pointer carrying a small window, rather than the generic four-way resize cursor.
public static class DockDragCursor
{
    public static Cursor Create()
    {
        const int size = 32;
        var visual = new DrawingVisual();
        using (var drawing = visual.RenderOpen())
        {
            drawing.DrawRoundedRectangle(new SolidColorBrush(Color.FromRgb(41, 50, 62)),
                new Pen(new SolidColorBrush(Color.FromRgb(255, 214, 0)), 1), new Rect(10.5, 9.5, 20, 17), 2, 2);
            drawing.DrawLine(new Pen(new SolidColorBrush(Color.FromRgb(255, 214, 0)), 1), new Point(11, 14.5), new Point(30, 14.5));
            drawing.DrawRectangle(new SolidColorBrush(Color.FromRgb(167, 175, 186)), null, new Rect(16, 18, 10, 1));
            drawing.DrawRectangle(new SolidColorBrush(Color.FromRgb(167, 175, 186)), null, new Rect(16, 21, 6, 1));
            drawing.DrawGeometry(Brushes.White, new Pen(Brushes.Black, 1.2),
                Geometry.Parse("M 2,1 L 2,20 L 6,16 L 10,24 L 13,22 L 9,15 L 16,15 Z"));
        }
        var bitmap = new RenderTargetBitmap(size, size, 96, 96, PixelFormats.Pbgra32);
        bitmap.Render(visual);
        var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(bitmap));
        using var png = new MemoryStream(); encoder.Save(png);
        using var stream = new MemoryStream();
        using (var writer = new BinaryWriter(stream, System.Text.Encoding.UTF8, leaveOpen: true))
        {
            // CUR directory: one PNG image, with its hotspot at the pointer tip.
            writer.Write((ushort)0); writer.Write((ushort)2); writer.Write((ushort)1);
            writer.Write((byte)size); writer.Write((byte)size); writer.Write((byte)0); writer.Write((byte)0);
            writer.Write((ushort)2); writer.Write((ushort)1);
            writer.Write((uint)png.Length); writer.Write((uint)22); writer.Write(png.ToArray());
        }
        stream.Position = 0;
        return new Cursor(stream, scaleWithDpi: true);
    }
}
