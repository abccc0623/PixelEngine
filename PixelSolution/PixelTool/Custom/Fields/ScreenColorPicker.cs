using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace PixelTool;

/// <summary>Local, memory-only desktop snapshot for click-to-pick. No files or network.</summary>
internal sealed class ScreenColorPicker : Window
{
    private readonly BitmapSource snapshot;
    private readonly int screenX, screenY;
    private readonly byte alpha;
    internal Color? SelectedColor { get; private set; }

    internal ScreenColorPicker(byte alpha)
    {
        this.alpha = alpha;
        screenX = GetSystemMetrics(76); screenY = GetSystemMetrics(77);
        int width = GetSystemMetrics(78), height = GetSystemMetrics(79);
        snapshot = Capture(screenX, screenY, width, height);
        Title = "스포이드 · 클릭하여 색상 선택 / Esc·우클릭 취소";
        WindowStyle = WindowStyle.None; ResizeMode = ResizeMode.NoResize;
        ShowInTaskbar = false; Topmost = true; WindowStartupLocation = WindowStartupLocation.Manual;
        Left = SystemParameters.VirtualScreenLeft; Top = SystemParameters.VirtualScreenTop;
        Width = SystemParameters.VirtualScreenWidth; Height = SystemParameters.VirtualScreenHeight;
        Cursor = Cursors.Cross; Background = Brushes.Black;
        var content = new Grid();
        content.Children.Add(new Image { Source = snapshot, Stretch = Stretch.Fill });
        var hint = new Border { Padding = new Thickness(14, 8, 14, 8), Margin = new Thickness(20), CornerRadius = new CornerRadius(4), HorizontalAlignment = HorizontalAlignment.Center, VerticalAlignment = VerticalAlignment.Bottom };
        hint.SetResourceReference(Border.BackgroundProperty, "PixelPanelBrush");
        var label = new TextBlock { Text = "화면을 클릭하여 색상 선택 · Esc 또는 우클릭으로 취소" };
        label.SetResourceReference(TextBlock.ForegroundProperty, "PixelTextBrush");
        hint.Child = label; content.Children.Add(hint); Content = content;
        void PositionWindow(object? sender, EventArgs e)
        {
            // Native screen pixels avoid DIP scaling errors and negative monitor coordinates.
            if (!SetWindowPos(new WindowInteropHelper(this).Handle, new IntPtr(-1), screenX, screenY, width, height, 0x0040))
                throw new Win32Exception(Marshal.GetLastWin32Error());
        }
        SourceInitialized += PositionWindow;
        Loaded += (sender, e) => PositionWindow(sender, e);
        PreviewMouseLeftButtonDown += (_, e) =>
        {
            if (!GetCursorPos(out var point)) return;
            SelectedColor = Sample(snapshot, point.X - screenX, point.Y - screenY, this.alpha);
            if (SelectedColor.HasValue) DialogResult = true;
            e.Handled = true;
        };
        PreviewMouseRightButtonDown += (_, e) => { e.Handled = true; Close(); };
        PreviewKeyDown += (_, e) => { if (e.Key == Key.Escape) { e.Handled = true; Close(); } };
        Deactivated += (_, _) => { if (IsVisible) Close(); };
        Closed += (_, _) => Content = null; // Drop the screenshot reference when the picker closes.
    }

    internal static Color? Sample(BitmapSource image, int x, int y, byte alpha)
    {
        if (x < 0 || y < 0 || x >= image.PixelWidth || y >= image.PixelHeight) return null;
        BitmapSource source = image.Format == PixelFormats.Bgra32 ? image : new FormatConvertedBitmap(image, PixelFormats.Bgra32, null, 0);
        byte[] pixel = new byte[4]; source.CopyPixels(new Int32Rect(x, y, 1, 1), pixel, 4, 0);
        return Color.FromArgb(alpha, pixel[2], pixel[1], pixel[0]);
    }

    private static BitmapSource Capture(int x, int y, int width, int height)
    {
        if (width <= 0 || height <= 0) throw new InvalidOperationException("사용 가능한 화면이 없습니다.");
        DwmFlush();
        IntPtr desktop = GetDC(IntPtr.Zero);
        if (desktop == IntPtr.Zero) throw new Win32Exception(Marshal.GetLastWin32Error());
        IntPtr memory = IntPtr.Zero, bitmap = IntPtr.Zero, previous = IntPtr.Zero;
        try
        {
            memory = CreateCompatibleDC(desktop);
            bitmap = CreateCompatibleBitmap(desktop, width, height);
            if (memory == IntPtr.Zero || bitmap == IntPtr.Zero) throw new Win32Exception(Marshal.GetLastWin32Error());
            previous = SelectObject(memory, bitmap);
            if (previous == IntPtr.Zero || previous == new IntPtr(-1)) throw new Win32Exception(Marshal.GetLastWin32Error());
            if (!BitBlt(memory, 0, 0, width, height, desktop, x, y, 0x40CC0020)) throw new Win32Exception(Marshal.GetLastWin32Error());
            var image = Imaging.CreateBitmapSourceFromHBitmap(bitmap, IntPtr.Zero, Int32Rect.Empty, BitmapSizeOptions.FromEmptyOptions());
            // GDI screenshots have no meaningful alpha. Normalize before sampling or displaying.
            var opaque = new FormatConvertedBitmap(image, PixelFormats.Bgr32, null, 0); opaque.Freeze();
            return opaque;
        }
        finally
        {
            if (previous != IntPtr.Zero && previous != new IntPtr(-1)) SelectObject(memory, previous);
            if (bitmap != IntPtr.Zero) DeleteObject(bitmap);
            if (memory != IntPtr.Zero) DeleteDC(memory);
            ReleaseDC(IntPtr.Zero, desktop);
        }
    }

    [StructLayout(LayoutKind.Sequential)] private struct CursorPoint { public int X, Y; }
    [DllImport("user32.dll")] private static extern int GetSystemMetrics(int index);
    [DllImport("user32.dll", SetLastError = true)] private static extern IntPtr GetDC(IntPtr window);
    [DllImport("user32.dll")] private static extern int ReleaseDC(IntPtr window, IntPtr dc);
    [DllImport("user32.dll", SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool GetCursorPos(out CursorPoint point);
    [DllImport("user32.dll", SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool SetWindowPos(IntPtr window, IntPtr after, int x, int y, int width, int height, uint flags);
    [DllImport("gdi32.dll", SetLastError = true)] private static extern IntPtr CreateCompatibleDC(IntPtr dc);
    [DllImport("gdi32.dll", SetLastError = true)] private static extern IntPtr CreateCompatibleBitmap(IntPtr dc, int width, int height);
    [DllImport("gdi32.dll", SetLastError = true)] private static extern IntPtr SelectObject(IntPtr dc, IntPtr obj);
    [DllImport("gdi32.dll", SetLastError = true)] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool BitBlt(IntPtr target, int x, int y, int width, int height, IntPtr source, int sourceX, int sourceY, uint operation);
    [DllImport("gdi32.dll")] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool DeleteObject(IntPtr obj);
    [DllImport("gdi32.dll")] [return: MarshalAs(UnmanagedType.Bool)] private static extern bool DeleteDC(IntPtr dc);
    [DllImport("dwmapi.dll")] private static extern int DwmFlush();
}
