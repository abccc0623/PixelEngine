using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using Microsoft.Win32;

namespace PixelTool;

public partial class SpriteEditor : UserControl, IDockCloseGuard
{
    private SpriteSheetDocument _document = new();
    private BitmapSource? _texture;
    private string? _path;
    private bool _ready, _filling, _dirty, _draftDirty, _dragging, _playing;
    private int _editing = -1, _frame;
    private Point _dragStart;
    private readonly ObservableCollection<RegionItem> _items = new();
    private readonly DispatcherTimer _timer = new() { Interval = TimeSpan.FromMilliseconds(16) };
    private readonly Stopwatch _clock = new();
    public sealed record RegionItem(SpriteRegion Region, BitmapSource Thumbnail)
    {
        public string Details => $"{Region.Width}×{Region.Height}" + (Region.Kind == SpriteRegionKind.Animation ? $" · {Region.Frames} frames" : " · Sprite");
    }

    public SpriteEditor()
    {
        InitializeComponent();
        foreach (var input in new[] { XInput, YInput, WidthInput, HeightInput, CountInput })
            input.LostMouseCapture += (_, _) => input.Maximum = input == CountInput ? 1024 : 8192;
        RegionList.ItemsSource = _items;
        _timer.Tick += (_, _) => Animate();
        Unloaded += (_, _) => { StopPlayback(); };
        IsVisibleChanged += (_, _) => { if (!IsVisible) { StopPlayback(); } };
        _ready = true;
        EditFields.IsEnabled = false;
        UpdateTitle();
    }

    private static BitmapSource ReadImage(string path)
    {
        using var stream = File.OpenRead(path);
        var bitmap = BitmapDecoder.Create(stream, BitmapCreateOptions.IgnoreImageCache, BitmapCacheOption.OnLoad).Frames[0];
        if (bitmap.PixelWidth > 8192 || bitmap.PixelHeight > 8192 || (long)bitmap.PixelWidth * bitmap.PixelHeight > 16777216)
            throw new InvalidDataException("이미지는 최대 8192축, 총 1600만 픽셀까지 지원합니다.");
        bitmap.Freeze();
        return bitmap;
    }

    private static string AssetPath(string path)
    {
        string absolute = Path.GetFullPath(path);
        AssetFileOperations.RequireAssetPath(absolute, ProjectPathService.AssetPath);
        return absolute;
    }

    public bool OpenDocument(string path)
    {
        try
        {
            path = AssetPath(path);
            var document = SpriteSheetDocument.Read(path);
            if (Path.IsPathRooted(document.Texture) || document.Texture.Contains(':')) throw new InvalidDataException("원본 이미지는 Asset 기준의 프로젝트 상대 경로여야 합니다.");
            string imagePath = AssetPath(Path.Combine(ProjectPathService.ProjectRootPath, document.Texture));
            var texture = ReadImage(imagePath);
            document.Validate(texture.PixelWidth, texture.PixelHeight);
            if (!ConfirmClose()) return false;
            SetDocument(document, texture, path);
            Status.Text = "파일을 열었습니다. 항목을 선택해 수정할 수 있습니다.";
            return true;
        }
        catch (Exception ex) { Status.Text = "열기 실패: " + ex.Message; return false; }
    }

    private void SetDocument(SpriteSheetDocument document, BitmapSource? texture, string? path)
    {
        StopPlayback();
        _document = document; _texture = texture; _path = path;
        _dirty = _draftDirty = false; _editing = -1;
        SheetImage.Source = texture;
        SheetImage.Stretch = Stretch.Fill;
        SheetCanvas.Width = SheetImage.Width = texture?.PixelWidth ?? 0;
        SheetCanvas.Height = SheetImage.Height = texture?.PixelHeight ?? 0;
        TextureInfo.Text = texture == null ? "이미지를 선택하고 영역을 지정하세요." : $"{document.Texture} · {texture.PixelWidth}×{texture.PixelHeight} px";
        EditFields.IsEnabled = texture != null;
        RefreshList();
        FillFields(document.Regions.FirstOrDefault() ?? new SpriteRegion
        { Width = Math.Min(32, texture?.PixelWidth ?? 32), Height = Math.Min(32, texture?.PixelHeight ?? 32) });
        if (_items.Count > 0) { _editing = 0; SetSelection(0); }
        UpdateTitle();
    }

    private void NewFile(object sender, RoutedEventArgs e)
    {
        if (ConfirmClose()) SetDocument(new(), null, null);
    }

    private void OpenFileClick(object sender, RoutedEventArgs e)
    {
        var picker = new OpenFileDialog { Filter = "Sprite sheet data|*.sprite", InitialDirectory = ProjectPathService.AssetPath };
        if (picker.ShowDialog() == true) OpenDocument(picker.FileName);
    }

    private void ChooseImage(object sender, RoutedEventArgs e)
    {
        var picker = new OpenFileDialog { Filter = "Images|*.png;*.jpg;*.jpeg;*.bmp", InitialDirectory = ProjectPathService.AssetPath };
        if (picker.ShowDialog() != true) return;
        try
        {
            string imagePath = AssetPath(picker.FileName);
            var texture = ReadImage(imagePath);
            if (!ConfirmClose()) return;
            SetDocument(new() { Texture = Path.GetRelativePath(ProjectPathService.ProjectRootPath, imagePath).Replace('\\', '/') }, texture, null);
            _dirty = true; UpdateTitle();
            Status.Text = "원본을 선택했습니다. 영역을 지정하고 추가 / 변경 적용을 누르세요.";
        }
        catch (Exception ex) { Status.Text = "이미지 선택 실패: " + ex.Message; }
    }

    private static int ReadInt(NumberField input) => input.Value == Math.Truncate(input.Value)
        ? checked((int)input.Value) : throw new InvalidDataException("좌표·크기·프레임 수는 정수로 입력하세요.");

    private SpriteRegion ReadDraft()
    {
        if (_texture == null) throw new InvalidDataException("원본 이미지를 먼저 선택하세요.");
        bool animation = AnimatedKind.IsChecked == true;
        var region = new SpriteRegion { Name = NameInput.Text.Trim(), Kind = animation ? SpriteRegionKind.Animation : SpriteRegionKind.Sprite,
            X = ReadInt(XInput), Y = ReadInt(YInput), Width = ReadInt(WidthInput), Height = ReadInt(HeightInput),
            Frames = animation ? ReadInt(CountInput) : 1, FrameSeconds = animation ? SecondsInput.Value : .1,
            Loop = LoopInput.Value };
        region.Validate(_texture.PixelWidth, _texture.PixelHeight);
        return region;
    }

    private void FillFields(SpriteRegion region)
    {
        _filling = true;
        ResetNumericBounds();
        NameInput.Text = region.Name; XInput.Value = region.X; YInput.Value = region.Y;
        WidthInput.Value = region.Width; HeightInput.Value = region.Height;
        CountInput.Value = region.Frames; SecondsInput.Value = region.FrameSeconds;
        StaticKind.IsChecked = region.Kind == SpriteRegionKind.Sprite; AnimatedKind.IsChecked = region.Kind == SpriteRegionKind.Animation;
        LoopInput.Value = region.Loop;
        AnimationFields.Visibility = region.Kind == SpriteRegionKind.Animation ? Visibility.Visible : Visibility.Collapsed;
        _filling = false; _draftDirty = false;
        UpdatePreview(); UpdateTitle();
    }

    private void DraftChanged(object sender, RoutedEventArgs e) => Changed();

    private (double Min, double Max, double Step) NumberRange(NumberField input)
    {
        if (input == SecondsInput) return (.001, 60, .01);
        int Value(NumberField field, int fallback) => Math.Clamp((int)field.Value, 0, 8192);
        int width = _texture?.PixelWidth ?? 8192, height = _texture?.PixelHeight ?? 8192;
        int x = Value(XInput, 0), y = Value(YInput, 0);
        int frames = AnimatedKind.IsChecked == true ? Math.Max(1, Value(CountInput, 1)) : 1;
        if (input == XInput) return (0, Math.Max(0, width - (double)Math.Max(1, Value(WidthInput, 1)) * frames), 1);
        if (input == YInput) return (0, Math.Max(0, height - Math.Max(1, Value(HeightInput, 1))), 1);
        if (input == WidthInput) return (1, Math.Max(1, (width - x) / frames), 1);
        if (input == HeightInput) return (1, Math.Max(1, height - y), 1);
        return (1, Math.Max(1, Math.Min(1024, (width - x) / Math.Max(1, Value(WidthInput, 1)))), 1);
    }
    private void PrepareNumberDrag(object sender, MouseButtonEventArgs e)
    {
        if (!_ready || _texture == null || sender is not NumberField input) return;
        // Re-evaluate the image bounds before the shared numeric field starts dragging.
        var range = NumberRange(input);
        input.Maximum = range.Max;
    }

    private void ResetNumericBounds()
    {
        foreach (var input in new[] { XInput, YInput, WidthInput, HeightInput }) input.Maximum = 8192;
        CountInput.Maximum = 1024;
    }
    private void LoopChanged(object sender, RoutedEventArgs e) => Changed();
    private void KindChanged(object sender, RoutedEventArgs e)
    {
        if (!_ready) return;
        AnimationFields.Visibility = AnimatedKind.IsChecked == true ? Visibility.Visible : Visibility.Collapsed;
        Changed();
    }
    private void Changed()
    {
        if (!_ready || _filling) return;
        _draftDirty = true; StopPlayback(); UpdateTitle(); UpdatePreview();
    }

    private BitmapSource Crop(SpriteRegion region, int frame = 0)
    {
        var crop = new CroppedBitmap(_texture!, new Int32Rect(region.X + frame * region.Width, region.Y, region.Width, region.Height));
        crop.Freeze(); return crop;
    }

    private void UpdatePreview()
    {
        if (_texture == null) { PreviewImage.Source = null; SelectionBox.Visibility = Visibility.Collapsed; return; }
        try
        {
            var region = ReadDraft();
            PreviewImage.Source = Crop(region, _frame = 0);
            FrameInfo.Text = region.Kind == SpriteRegionKind.Animation ? $"1 / {region.Frames}" : "일반 스프라이트";
            Canvas.SetLeft(SelectionBox, region.X); Canvas.SetTop(SelectionBox, region.Y);
            SelectionBox.Width = region.Width * region.Frames; SelectionBox.Height = region.Height;
            SelectionBox.Visibility = Visibility.Visible;
            Status.Text = "선택 영역: " + region.X + ", " + region.Y + " · " + region.Width + "×" + region.Height;
        }
        catch (InvalidDataException ex) { PreviewImage.Source = null; SelectionBox.Visibility = Visibility.Collapsed; Status.Text = ex.Message; }
        DrawGrid();
    }

    private bool ApplyDraft()
    {
        try
        {
            var region = ReadDraft();
            if (_document.Regions.Where((_, i) => i != _editing).Any(r => r.Name.Equals(region.Name, StringComparison.OrdinalIgnoreCase)))
                throw new InvalidDataException("같은 이름이 있습니다. 다른 이름을 입력하세요.");
            if (_editing < 0)
            {
                if (_document.Regions.Count >= 1024) throw new InvalidDataException("파일당 최대 1024개 항목을 추가할 수 있습니다.");
                _document.Regions.Add(region); _editing = _document.Regions.Count - 1;
            }
            else _document.Regions[_editing] = region;
            _dirty = true; _draftDirty = false;
            RefreshList(); SetSelection(_editing); UpdateTitle();
            Status.Text = "목록에 적용했습니다. 파일 저장을 눌러 디스크에 저장하세요.";
            return true;
        }
        catch (InvalidDataException ex) { Status.Text = ex.Message; return false; }
    }
    private void ApplyRegion(object sender, RoutedEventArgs e) => ApplyDraft();

    private bool ResolveDraft()
    {
        if (!_draftDirty) return true;
        var answer = PixelMessageBox.Show("편집 중인 항목의 변경을 목록에 적용할까요?", "Sprite Editor", MessageBoxButton.YesNoCancel, MessageBoxImage.Question);
        return answer == MessageBoxResult.No || (answer == MessageBoxResult.Yes && ApplyDraft());
    }

    private void RefreshList()
    {
        _filling = true; _items.Clear();
        if (_texture != null) foreach (var region in _document.Regions) _items.Add(new(region, Crop(region)));
        _filling = false;
    }
    private void SetSelection(int index) { _filling = true; RegionList.SelectedIndex = index; _filling = false; }
    private void RegionSelected(object sender, SelectionChangedEventArgs e)
    {
        if (!_ready || _filling || RegionList.SelectedIndex < 0) return;
        int index = RegionList.SelectedIndex;
        if (!ResolveDraft()) { SetSelection(_editing); return; }
        _editing = index; FillFields(_document.Regions[index]); SetSelection(index);
    }
    private string UniqueName(string name)
    {
        string candidate = name;
        for (int i = 1; _document.Regions.Any(r => r.Name.Equals(candidate, StringComparison.OrdinalIgnoreCase)); i++) candidate = name + "_" + i;
        return candidate;
    }
    private void NewRegion(object sender, RoutedEventArgs e)
    {
        if (!ResolveDraft()) return;
        _editing = -1; SetSelection(-1); FillFields(new() { Name = UniqueName("Sprite"), Width = Math.Min(32, _texture!.PixelWidth), Height = Math.Min(32, _texture.PixelHeight) });
    }
    private void DuplicateRegion(object sender, RoutedEventArgs e)
    {
        if (_texture == null || _editing < 0 || !ResolveDraft()) return;
        var copy = _document.Regions[_editing] with { Name = UniqueName(_document.Regions[_editing].Name + "_Copy") };
        _editing = -1; SetSelection(-1); FillFields(copy); _draftDirty = true;
        ApplyDraft();
    }
    private void DeleteRegion(object sender, RoutedEventArgs e)
    {
        if (_editing < 0 || !ResolveDraft()) return;
        _document.Regions.RemoveAt(_editing); _dirty = true; _editing = -1; _draftDirty = false;
        RefreshList(); FillFields(new() { Name = UniqueName("Sprite") }); UpdateTitle();
    }

    private void TogglePlay(object sender, RoutedEventArgs e)
    {
        if (_playing) { StopPlayback(); return; }
        try
        {
            var region = ReadDraft();
            if (region.Kind != SpriteRegionKind.Animation) return;
            _clock.Restart(); _playing = true; PlayButton.Content = "정지"; _timer.Start();
        }
        catch (InvalidDataException ex) { Status.Text = ex.Message; }
    }
    private void StopPlayback() { _timer.Stop(); _clock.Stop(); _playing = false; if (_ready) PlayButton.Content = "재생"; }
    private void Animate()
    {
        if (!_playing || !IsVisible) { StopPlayback(); return; }
        try
        {
            var region = ReadDraft();
            int elapsed = (int)Math.Min(int.MaxValue, _clock.Elapsed.TotalSeconds / region.FrameSeconds);
            int frame = region.Loop ? elapsed % region.Frames : Math.Min(elapsed, region.Frames - 1);
            if (_frame != frame) { _frame = frame; PreviewImage.Source = Crop(region, frame); FrameInfo.Text = $"{frame + 1} / {region.Frames}"; }
            if (!region.Loop && elapsed >= region.Frames) StopPlayback();
        }
        catch (InvalidDataException) { StopPlayback(); }
    }

    private void ZoomChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
    {
        if (!_ready) return;
        SheetScale.ScaleX = SheetScale.ScaleY = Zoom.Value; ZoomText.Text = $"{Zoom.Value * 100:0}%";
    }
    private void GridChanged(object sender, RoutedEventArgs e) { if (_ready) DrawGrid(); }
    private void DrawGrid()
    {
        GridLines.Data = null;
        if (_texture == null || !ShowGrid.Value || WidthInput.Value % 1 != 0 || HeightInput.Value % 1 != 0) return;
        int w = ReadInt(WidthInput), h = ReadInt(HeightInput);
        var geometry = new StreamGeometry();
        using (var context = geometry.Open())
        {
            for (int x = 0; x <= _texture.PixelWidth; x += w) { context.BeginFigure(new(x, 0), false, false); context.LineTo(new(x, _texture.PixelHeight), true, false); }
            for (int y = 0; y <= _texture.PixelHeight; y += h) { context.BeginFigure(new(0, y), false, false); context.LineTo(new(_texture.PixelWidth, y), true, false); }
        }
        geometry.Freeze(); GridLines.Data = geometry;
    }
    private Point PixelPoint(MouseEventArgs e)
    {
        var point = e.GetPosition(SheetCanvas);
        return new(Math.Clamp((int)point.X, 0, _texture!.PixelWidth - 1), Math.Clamp((int)point.Y, 0, _texture.PixelHeight - 1));
    }
    private void SheetDown(object sender, MouseButtonEventArgs e)
    {
        if (_texture == null || WidthInput.Value % 1 != 0 || HeightInput.Value % 1 != 0) return;
        _dragStart = PixelPoint(e); _dragging = true; SheetCanvas.CaptureMouse();
        int w = ReadInt(WidthInput), h = ReadInt(HeightInput);
        if (AnimatedKind.IsChecked == true && w > 0 && h > 0)
            _dragStart = new(Math.Floor(_dragStart.X / w) * w, Math.Floor(_dragStart.Y / h) * h);
        SetCoordinates((int)_dragStart.X, (int)_dragStart.Y, null, null, null);
        e.Handled = true;
    }
    private void SheetMove(object sender, MouseEventArgs e)
    {
        if (!_dragging || _texture == null || e.LeftButton != MouseButtonState.Pressed || WidthInput.Value % 1 != 0) return;
        var end = PixelPoint(e);
        int w = ReadInt(WidthInput);
        if (AnimatedKind.IsChecked == true && w > 0)
        {
            int count = Math.Min((_texture.PixelWidth - (int)_dragStart.X) / w, Math.Max(1, (int)(end.X - _dragStart.X) / w + 1));
            SetCoordinates((int)_dragStart.X, (int)_dragStart.Y, null, null, count);
        }
        else SetCoordinates((int)Math.Min(end.X, _dragStart.X), (int)Math.Min(end.Y, _dragStart.Y),
            (int)Math.Abs(end.X - _dragStart.X) + 1, (int)Math.Abs(end.Y - _dragStart.Y) + 1, null);
    }
    private void SetCoordinates(int x, int y, int? width, int? height, int? count)
    {
        _filling = true; ResetNumericBounds(); XInput.Value = x; YInput.Value = y;
        if (width.HasValue) WidthInput.Value = width.Value; if (height.HasValue) HeightInput.Value = height.Value;
        if (count.HasValue) CountInput.Value = count.Value; _filling = false; Changed();
    }
    private void SheetUp(object sender, MouseButtonEventArgs e) { _dragging = false; SheetCanvas.ReleaseMouseCapture(); }
    private void SheetLostCapture(object sender, MouseEventArgs e) => _dragging = false;

    private void SaveClick(object sender, RoutedEventArgs e) => SaveDocument(false);
    private void SaveAsClick(object sender, RoutedEventArgs e) => SaveDocument(true);
    public bool SaveDocument(bool saveAs)
    {
        try
        {
            if (_texture == null) throw new InvalidDataException("원본 이미지를 먼저 선택하세요.");
            if (_draftDirty && !ApplyDraft()) return false;
            _document.Validate(_texture.PixelWidth, _texture.PixelHeight);
            string? path = _path;
            if (saveAs || path == null)
            {
                var picker = new SaveFileDialog { Filter = "Sprite sheet data|*.sprite", DefaultExt = ".sprite", AddExtension = true,
                    InitialDirectory = ProjectPathService.AssetPath, FileName = Path.GetFileNameWithoutExtension(_document.Texture) + ".sprite" };
                if (picker.ShowDialog() != true) return false;
                path = picker.FileName;
            }
            path = AssetPath(path); _document.Save(path); _path = path; _dirty = false; UpdateTitle();
            GlobalFunction.GetDockedWindow<AssetWindow>()?.Refresh();
            Status.Text = "저장 완료: " + Path.GetRelativePath(ProjectPathService.ProjectRootPath, path).Replace('\\', '/');
            return true;
        }
        catch (Exception ex) { Status.Text = "저장 실패: " + ex.Message; return false; }
    }
    public bool ConfirmClose()
    {
        if (!_dirty && !_draftDirty) return true;
        var answer = PixelMessageBox.Show("스프라이트 파일의 변경을 저장할까요?", "Sprite Editor", MessageBoxButton.YesNoCancel, MessageBoxImage.Question);
        return answer == MessageBoxResult.No || (answer == MessageBoxResult.Yes && SaveDocument(false));
    }
    private void UpdateTitle()
    {
        string title = (_path == null ? "새 스프라이트" : Path.GetFileName(_path)) + (_dirty || _draftDirty ? " *" : "");
        FileTitle.Text = title; Tag = title;
    }
}
