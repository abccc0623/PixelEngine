using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Microsoft.Win32;

namespace PixelTool;

/// <summary>Unsaved image preview. Only an explicit save writes into the project.</summary>
public sealed class ArtImageResultWindow : Window
{
    private readonly byte[] _png;
    private readonly string _assetRoot, _suggestedName;
    private readonly TextBlock _status = new() { Margin = new Thickness(0, 8, 0, 8), TextWrapping = TextWrapping.Wrap };

    public ArtImageResultWindow(byte[] png, string assetRoot, string suggestedName)
    {
        _png = (byte[])png.Clone(); _assetRoot = assetRoot; _suggestedName = suggestedName;
        using var stream = new MemoryStream(_png, false);
        var bitmap = DecodeImage(stream);
        Title = "이미지 생성 결과"; Width = 640; Height = 580; MinWidth = 360; MinHeight = 320;
        WindowStartupLocation = WindowStartupLocation.CenterOwner;
        Resources.MergedDictionaries.Add(new ResourceDictionary { Source = new Uri("/PixelTool;component/Custom/Fields/FieldStyles.xaml", UriKind.Relative) });
        SetResourceReference(BackgroundProperty, "PixelBackgroundBrush");
        SetResourceReference(ForegroundProperty, "PixelTextBrush");
        var root = new DockPanel { Margin = new Thickness(16) };
        var footer = new StackPanel(); DockPanel.SetDock(footer, Dock.Bottom);
        _status.Text = $"{bitmap.PixelWidth} × {bitmap.PixelHeight} px · 아직 저장하지 않았습니다.";
        _status.SetResourceReference(TextBlock.ForegroundProperty, "PixelTextBrush");
        footer.Children.Add(_status);
        var buttons = new StackPanel { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Right };
        var save = new Button { Content = "Asset에 저장", Padding = new Thickness(14, 8, 14, 8), Style = (Style)FindResource("FieldButton"), Margin = new Thickness(0, 0, 8, 0) };
        var discard = new Button { Content = "저장 안 함", Padding = new Thickness(14, 8, 14, 8), Style = (Style)FindResource("FieldButton") };
        save.Click += SaveClick;
        discard.Click += (_, _) => Close();
        buttons.Children.Add(save); buttons.Children.Add(discard); footer.Children.Add(buttons); root.Children.Add(footer);
        var image = new Image { Source = bitmap, Stretch = Stretch.Uniform, Margin = new Thickness(12) };
        RenderOptions.SetBitmapScalingMode(image, BitmapScalingMode.NearestNeighbor);
        var preview = new Border { Background = (Brush)FindResource("FieldChecker"), Child = image, BorderThickness = new Thickness(1) };
        preview.SetResourceReference(Border.BorderBrushProperty, "PixelBorderBrush"); root.Children.Add(preview);
        Content = root;
    }

    private void SaveClick(object sender, RoutedEventArgs e)
    {
        try
        {
            if (!Directory.Exists(_assetRoot)) throw new IOException("원래 프로젝트의 Asset 폴더를 찾을 수 없습니다.");
            string suggested = Path.GetFileName(AssetFileOperations.UniquePath(_assetRoot, Path.GetFileNameWithoutExtension(_suggestedName), ".png"));
            var picker = new SaveFileDialog { InitialDirectory = _assetRoot, FileName = suggested, Filter = "PNG 이미지|*.png", DefaultExt = ".png", AddExtension = true, OverwritePrompt = false };
            if (picker.ShowDialog(this) != true) return;
            SaveImage(picker.FileName, _assetRoot, _png);
            _status.Text = "저장 완료: " + picker.FileName;
            MessageBox.Show(this, "이미지를 저장했습니다.\n" + picker.FileName, "Art Studio", MessageBoxButton.OK, MessageBoxImage.Information);
            Close();
        }
        catch (Exception ex) when (ex is IOException or ArgumentException or UnauthorizedAccessException or NotSupportedException)
        { _status.Text = "저장 실패: " + ex.Message; }
    }

    internal static void SaveImage(string path, string assetRoot, byte[] png)
    {
        AssetFileOperations.RequireAssetPath(path, assetRoot);
        if (!Path.GetExtension(path).Equals(".png", StringComparison.OrdinalIgnoreCase)) throw new IOException("PNG 파일로 저장해 주세요.");
        using (var source = new MemoryStream(png, false)) DecodeImage(source);
        // Never overwrite a project asset, including a collision after the save dialog opened.
        bool created = false;
        try
        {
            using var output = new FileStream(path, FileMode.CreateNew, FileAccess.Write, FileShare.None);
            created = true; output.Write(png); output.Flush();
        }
        catch
        {
            if (created) File.Delete(path);
            throw;
        }
    }

    internal static BitmapSource DecodeImage(Stream stream)
    {
        var decoder = BitmapDecoder.Create(stream, BitmapCreateOptions.PreservePixelFormat, BitmapCacheOption.OnLoad);
        var frame = decoder.Frames[0];
        if (frame.PixelWidth > 8192 || frame.PixelHeight > 8192 || (long)frame.PixelWidth * frame.PixelHeight > 16777216)
            throw new IOException("이미지가 너무 큽니다. 최대 한 변 8192px, 총 1,677만 픽셀까지 사용할 수 있습니다.");
        frame.Freeze(); return frame;
    }

    internal static byte[] EncodePng(BitmapSource bitmap)
    {
        var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(bitmap));
        using var stream = new MemoryStream(); encoder.Save(stream); return stream.ToArray();
    }

    internal static byte[] FitImage(byte[] png, int width, int height)
    {
        if (width < 1 || height < 1 || width > 8192 || height > 8192 || (long)width * height > 16777216)
            throw new ArgumentOutOfRangeException(nameof(width));
        using var stream = new MemoryStream(png, false);
        var source = DecodeImage(stream);
        var visual = new DrawingVisual(); RenderOptions.SetBitmapScalingMode(visual, BitmapScalingMode.NearestNeighbor);
        double scale = Math.Min((double)width / source.PixelWidth, (double)height / source.PixelHeight);
        double w = source.PixelWidth * scale, h = source.PixelHeight * scale;
        using (var drawing = visual.RenderOpen()) drawing.DrawImage(source, new Rect((width - w) / 2, (height - h) / 2, w, h));
        var result = new RenderTargetBitmap(width, height, 96, 96, PixelFormats.Pbgra32);
        result.Render(visual); result.Freeze();
        return EncodePng(result);
    }
}
