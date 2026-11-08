using System.Windows;

namespace PixelTool;

// Shared geometry for guide rendering and release hit testing (WPF logical pixels).
public static class DockGuideLayout
{
    public const double Size = 112;
    public static readonly string[] Directions = { "Top", "Left", "Tab", "Right", "Bottom" };
    public static double Scale(Rect pane) => Math.Min(1, Math.Min(pane.Width, pane.Height) / Size);
    public static Rect IconBounds(Rect pane, string direction)
    {
        double scale = Scale(pane);
        double x = direction == "Left" ? -38 : direction == "Right" ? 38 : 0;
        double y = direction == "Top" ? -38 : direction == "Bottom" ? 38 : 0;
        return new Rect(pane.X + pane.Width / 2 + (x - 16) * scale,
            pane.Y + pane.Height / 2 + (y - 16) * scale, 32 * scale, 32 * scale);
    }
    public static string? HitTest(Rect pane, Point point)
    {
        if (!pane.Contains(point) || pane.Width < 1 || pane.Height < 1) return null;
        return Directions.FirstOrDefault(direction => IconBounds(pane, direction).Contains(point));
    }
    public static Rect PreviewBounds(Rect pane, string direction)
    {
        if (direction == "Left") pane.Width /= 2;
        if (direction == "Right") { pane.X += pane.Width / 2; pane.Width /= 2; }
        if (direction == "Top") pane.Height /= 2;
        if (direction == "Bottom") { pane.Y += pane.Height / 2; pane.Height /= 2; }
        return pane;
    }
}
