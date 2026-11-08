using System.IO;
using System.Windows;
using System.Windows.Controls;
using Microsoft.Win32;

namespace PixelTool;

/// <summary>Selects an existing project Asset file without copying or moving it.</summary>
public sealed class AssetField : EditorFieldBase
{
    public static readonly DependencyProperty AssetPathProperty = DependencyProperty.Register(nameof(AssetPath), typeof(string), typeof(AssetField),
        new FrameworkPropertyMetadata("", FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, (d, _) => ((AssetField)d).Refresh()));
    public static readonly DependencyProperty AllowedExtensionsProperty = DependencyProperty.Register(nameof(AllowedExtensions), typeof(string), typeof(AssetField), new PropertyMetadata(""));
    public string AssetPath { get => (string)GetValue(AssetPathProperty); set => SetValue(AssetPathProperty, value); }
    public string AllowedExtensions { get => (string)GetValue(AllowedExtensionsProperty); set => SetValue(AllowedExtensionsProperty, value); }
    public string? Error { get; private set; }
    private readonly TextBlock pathText = new() { VerticalAlignment = VerticalAlignment.Center, TextTrimming = TextTrimming.CharacterEllipsis, Margin = new Thickness(6, 0, 4, 0) };

    public AssetField()
    {
        InputBorder.MinWidth = 128;
        var grid = new Grid();
        grid.ColumnDefinitions.Add(new ColumnDefinition());
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(24) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(24) });
        pathText.SetResourceReference(TextBlock.ForegroundProperty, "PixelTextBrush");
        grid.Children.Add(pathText);
        var select = new Button { Content = "…", ToolTip = "Asset 파일 선택", Style = FieldStyle("FieldButton") };
        var clear = new Button { Content = "×", ToolTip = "참조 비우기", Style = FieldStyle("FieldButton") };
        Grid.SetColumn(select, 1); Grid.SetColumn(clear, 2);
        grid.Children.Add(select); grid.Children.Add(clear);
        select.Click += (_, _) =>
        {
            if (!Directory.Exists(ProjectPathService.AssetPath)) { ToolTip = Error = "프로젝트 Asset 폴더를 먼저 열어 주세요."; return; }
            var dialog = new OpenFileDialog { InitialDirectory = ProjectPathService.AssetPath, CheckFileExists = true, Multiselect = false, Filter = "Asset files|*.*" };
            if (dialog.ShowDialog() == true) TrySetAsset(dialog.FileName);
        };
        clear.Click += (_, _) => SetCurrentValue(AssetPathProperty, "");
        AllowDrop = true;
        PreviewDragOver += (sender, e) =>
        {
            string[]? files = ReadFiles(e.Data);
            e.Effects = files?.Length == 1 && Validate(files[0], out _, out _) ? DragDropEffects.Copy : DragDropEffects.None;
            e.Handled = true;
        };
        PreviewDrop += (_, e) =>
        {
            string[]? files = ReadFiles(e.Data);
            if (files?.Length == 1) TrySetAsset(files[0]);
            e.Handled = true;
        };
        EditorHost.Content = grid;
        Refresh();
    }

    public bool TrySetAsset(string path)
    {
        if (!Validate(path, out string normalized, out string? error)) { ToolTip = Error = error; return false; }
        SetCurrentValue(AssetPathProperty, normalized);
        return true;
    }

    private bool Validate(string path, out string normalized, out string? error)
    {
        normalized = ""; error = null;
        try
        {
            if (string.IsNullOrWhiteSpace(ProjectPathService.ProjectRootPath)) throw new IOException("프로젝트를 먼저 열어 주세요.");
            string full = Path.GetFullPath(Path.IsPathRooted(path) ? path : Path.Combine(ProjectPathService.ProjectRootPath, path));
            AssetFileOperations.RequireAssetPath(full, ProjectPathService.AssetPath);
            if (!File.Exists(full)) throw new IOException("Asset 안에 있는 파일을 선택해 주세요.");
            var extensions = (AllowedExtensions ?? "").Split(new[] { ';', ',', ' ' }, StringSplitOptions.RemoveEmptyEntries);
            if (extensions.Length > 0 && !extensions.Any(ext => Path.GetExtension(full).Equals("." + ext.TrimStart('*', '.'), StringComparison.OrdinalIgnoreCase)))
                throw new IOException("허용된 파일 형식이 아닙니다.");
            normalized = Path.GetRelativePath(ProjectPathService.ProjectRootPath, full).Replace('\\', '/');
            return true;
        }
        catch (Exception ex) when (ex is IOException or ArgumentException or UnauthorizedAccessException or NotSupportedException)
        { error = ex.Message; return false; }
    }

    private static string[]? ReadFiles(IDataObject data) => data.GetData("PixelTool.AssetPaths") as string[] ?? data.GetData(DataFormats.FileDrop) as string[];
    private void Refresh()
    {
        pathText.Text = string.IsNullOrEmpty(AssetPath) ? "None" : AssetPath;
        if (string.IsNullOrEmpty(AssetPath)) Error = null;
        else { Validate(AssetPath, out _, out string? error); Error = error; }
        ToolTip = Error ?? AssetPath;
        NotifyValueChanged();
    }
}
