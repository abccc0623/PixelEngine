using System.Windows;
using PixelTool;

void Require(bool value, string message) { if (!value) throw new Exception(message); }
foreach (var pane in new[] { new Rect(0, 0, 800, 600), new Rect(200, 100, 320, 180), new Rect(-400, 40, 160, 70) })
{
    foreach (string direction in DockGuideLayout.Directions)
    {
        Rect icon = DockGuideLayout.IconBounds(pane, direction);
        var center = new Point(icon.X + icon.Width / 2, icon.Y + icon.Height / 2);
        Require(DockGuideLayout.HitTest(pane, center) == direction, "Incorrect guide target: " + direction);
        Rect preview = DockGuideLayout.PreviewBounds(pane, direction);
        Require(pane.Contains(preview), "Preview must stay inside the target pane.");
        Require(preview.Width == (direction is "Left" or "Right" ? pane.Width / 2 : pane.Width), "Incorrect preview width.");
        Require(preview.Height == (direction is "Top" or "Bottom" ? pane.Height / 2 : pane.Height), "Incorrect preview height.");
    }
    foreach (var point in new[] { new Point(pane.Left + 1, pane.Top + 1), new Point(pane.Right - 1, pane.Bottom - 1),
        new Point(pane.Left + 1, pane.Top + pane.Height / 2), new Point(pane.Right - 1, pane.Top + pane.Height / 2),
        new Point(pane.Left + pane.Width / 2 + 19 * DockGuideLayout.Scale(pane), pane.Top + pane.Height / 2) })
        Require(DockGuideLayout.HitTest(pane, point) == null, "Edges, blank space and gaps must not dock.");
    Require(DockGuideLayout.HitTest(pane, new Point(pane.Left - 1, pane.Top)) == null, "Outside the pane must not dock.");
}
Require(DockGuideLayout.HitTest(new Rect(0, 0, 0, 0), new Point()) == null, "Empty panes must not dock.");
Exception? cursorError = null;
var thread = new Thread(() =>
{
    try { using var cursor = DockDragCursor.Create(); Require(cursor != null, "Window-carrying cursor must load."); }
    catch (Exception ex) { cursorError = ex; }
});
thread.SetApartmentState(ApartmentState.STA); thread.Start(); thread.Join();
if (cursorError != null) throw cursorError;
Console.WriteLine("PASS: all five icon targets, blank/edge/gap releases, placement previews and actual custom window-drag cursor creation. No editor/game launched.");
