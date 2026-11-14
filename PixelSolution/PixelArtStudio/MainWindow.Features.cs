using Microsoft.Win32;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;

namespace PixelArtStudio;

public partial class MainWindow
{
    private string? currentMetadata;
    private string? sourceImage;
    private string? processedImage;
    private string? pinnedReference;
    private string SelectedCategory => CategoryBox.SelectedIndex switch { 1 => "background", 2 => "tile", _ => "character" };
    private void Category_Changed(object sender, SelectionChangedEventArgs e)
    {
        if (TransparentBox is not null) TransparentBox.IsChecked = CategoryBox.SelectedIndex == 0;
    }
    private StudioSettings CaptureSettings() => new()
    {
        StylePrompt = StylePromptBox.Text, StyleNegativePrompt = StyleNegativeBox.Text, Prompt = PromptBox.Text, Edit = EditBox.Text, NegativePrompt = NegativeBox.Text,
        Category = SelectedCategory, Steps = StepsBox.Text, Seed = SeedBox.Text,
        SizeIndex = SizeBox.SelectedIndex, Transparent = TransparentBox.IsChecked == true, ProtectInterior = ProtectInteriorBox.IsChecked == true,
        Strength = StrengthSlider.Value, ReferenceImage = pinnedReference ?? currentImage
    };
    private async void SaveSettings_Click(object sender, RoutedEventArgs e)
    {
        var d = new SaveFileDialog { Filter = "Studio settings|*.json", FileName = "settings.json" };
        if (d.ShowDialog(this) != true) return;
        try { await File.WriteAllTextAsync(d.FileName, JsonSerializer.Serialize(CaptureSettings(), JsonOptions)); Status.Text = "설정 저장 완료"; }
        catch (Exception ex) { Fail(ex.Message); }
    }
    private void LoadSettings_Click(object sender, RoutedEventArgs e)
    {
        var d = new OpenFileDialog { Filter = "Studio settings|*.json" };
        if (d.ShowDialog(this) != true) return;
        try
        {
            if (new FileInfo(d.FileName).Length > 1024 * 1024) throw new InvalidDataException("설정 파일은 1MB 이하만 지원합니다.");
            var s = JsonSerializer.Deserialize<StudioSettings>(File.ReadAllText(d.FileName), new JsonSerializerOptions { PropertyNameCaseInsensitive = true }) ?? throw new InvalidDataException("빈 설정입니다.");
            if (s.SizeIndex is < 0 or > 2 || s.Strength < .1 || s.Strength > 1 || !double.IsFinite(s.Strength) || s.Category is not ("character" or "background" or "tile") || !int.TryParse(s.Steps, out int steps) || steps is < 4 or > 50 || !long.TryParse(s.Seed, out long seed) || seed < -1 || seed > uint.MaxValue) throw new InvalidDataException("설정 값의 범위가 올바르지 않습니다.");
            if (!string.IsNullOrEmpty(s.ReferenceImage) && !File.Exists(s.ReferenceImage)) throw new FileNotFoundException("기준 이미지가 없습니다.", s.ReferenceImage);
            if (s.StylePrompt is null || s.StyleNegativePrompt is null || s.StylePrompt.Length > 2000 || s.StyleNegativePrompt.Length > 2000) throw new InvalidDataException("스타일 설명은 2000자 이하 문자열이어야 합니다.");
            StylePromptBox.Text = s.StylePrompt; StyleNegativeBox.Text = s.StyleNegativePrompt; PromptBox.Text = s.Prompt; EditBox.Text = s.Edit; NegativeBox.Text = s.NegativePrompt;
            CategoryBox.SelectedIndex = s.Category switch { "background" => 1, "tile" => 2, _ => 0 };
            StepsBox.Text = s.Steps; SeedBox.Text = s.Seed; SizeBox.SelectedIndex = s.SizeIndex;
            TransparentBox.IsChecked = s.Transparent; ProtectInteriorBox.IsChecked = s.ProtectInterior; StrengthSlider.Value = s.Strength;
            pinnedReference = s.ReferenceImage; ReferenceInfo.Text = pinnedReference ?? "기준 없음";
            appliedWorkItem = null; WorkItemCriteria.Text = "적용된 일감 없음 (설정 복원)";
            Status.Text = "설정 복원 완료";
        }
        catch (Exception ex) { Fail(ex.Message); }
    }
    private void ShowVariant(string? path)
    {
        if (path is null || !File.Exists(path)) { Fail("현재 이미지의 비교 원본/결과가 없습니다."); return; }
        string? source = sourceImage, processed = processedImage;
        try { LoadImage(path); sourceImage = source; processedImage = processed; Status.Text = "선택 이미지 복구 완료 · 내보내기에 반영됩니다."; }
        catch (Exception ex) { Fail(ex.Message); }
    }
    private void ExportMetadata_Click(object sender, RoutedEventArgs e)
    {
        if (currentMetadata is null || !File.Exists(currentMetadata)) { Fail("현재 미리보기의 시트 좌표 JSON이 없습니다."); return; }
        var d = new SaveFileDialog { Filter = "시트 좌표 JSON|*.json", FileName = "sheet.json", OverwritePrompt = true };
        if (d.ShowDialog(this) != true) return;
        try { if (!Path.GetFullPath(d.FileName).Equals(Path.GetFullPath(currentMetadata), StringComparison.OrdinalIgnoreCase)) File.Copy(currentMetadata, d.FileName, true); Status.Text = "시트 좌표 JSON 저장 완료"; }
        catch (Exception ex) { Fail(ex.Message); }
    }
    private void Source_Click(object sender, RoutedEventArgs e) => ShowVariant(sourceImage);
    private void Processed_Click(object sender, RoutedEventArgs e) => ShowVariant(processedImage);
    private void PinReference_Click(object sender, RoutedEventArgs e)
    {
        if (currentImage is null) { Fail("먼저 이미지를 여세요."); return; }
        pinnedReference = currentImage; ReferenceInfo.Text = pinnedReference;
    }
    private async void GenerateFrame_Click(object sender, RoutedEventArgs e)
    {
        if (pinnedReference is null || !File.Exists(pinnedReference)) { Fail("기준 이미지를 먼저 고정하세요."); return; }
        ClearWorkItemForDirectGeneration();
        await GenerateAsync(true, pinnedReference);
    }
    private void AddFrame_Click(object sender, RoutedEventArgs e)
    {
        if (currentImage is not null && FramesBox.Items.Count < 64) FramesBox.Items.Add(currentImage);
        else Fail("이미지가 없거나 최대 64프레임에 도달했습니다.");
    }
    private void RemoveFrame_Click(object sender, RoutedEventArgs e) { if (FramesBox.SelectedIndex >= 0) FramesBox.Items.RemoveAt(FramesBox.SelectedIndex); }
    private void MoveFrame(int delta)
    {
        int i = FramesBox.SelectedIndex, next = i + delta;
        if (i < 0 || next < 0 || next >= FramesBox.Items.Count) return;
        object frame = FramesBox.Items[i]; FramesBox.Items.RemoveAt(i); FramesBox.Items.Insert(next, frame); FramesBox.SelectedIndex = next;
    }
    private void FrameUp_Click(object sender, RoutedEventArgs e) => MoveFrame(-1);
    private void FrameDown_Click(object sender, RoutedEventArgs e) => MoveFrame(1);
    private async void Flip_Click(object sender, RoutedEventArgs e) => await RunImageOperation("flip");
    private async void Recolor_Click(object sender, RoutedEventArgs e) => await RunImageOperation("recolor");
    private async void Sheet_Click(object sender, RoutedEventArgs e) => await RunImageOperation("sheet");
    private async Task RunImageOperation(string action)
    {
        if (busy) return;
        if (action == "sheet" ? FramesBox.Items.Count == 0 : currentImage is null) { Fail("처리할 이미지/프레임이 없습니다."); return; }
        string python = PythonBox.Text.Trim(), script = Path.Combine(AppContext.BaseDirectory, "Backend", "image_ops.py");
        if (!File.Exists(python) || !File.Exists(script)) { Fail("이미지 처리 Python/스크립트를 찾을 수 없습니다."); return; }
        string dir = Path.Combine(dataRoot, "outputs", "edit-" + Guid.NewGuid().ToString("N"));
        string? output = null, metadata = null, error = null;
        var request = new { action, inputPath = currentImage, outputPath = Path.Combine(dir, action + ".png"), axis = "horizontal", fromColor = FromColorBox.Text.Trim(), toColor = ToColorBox.Text.Trim(), frames = FramesBox.Items.Cast<string>().ToArray() };
        SetBusy(true); cancellation = new CancellationTokenSource();
        try
        {
            Directory.CreateDirectory(dir); string json = Path.Combine(dir, "request.json");
            await File.WriteAllTextAsync(json, JsonSerializer.Serialize(request, JsonOptions));
            var start = new ProcessStartInfo(python) { UseShellExecute = false, CreateNoWindow = true, RedirectStandardOutput = true, RedirectStandardError = true, StandardOutputEncoding = Encoding.UTF8, StandardErrorEncoding = Encoding.UTF8 };
            start.ArgumentList.Add(script); start.ArgumentList.Add("--request"); start.ArgumentList.Add(json); start.Environment["PYTHONIOENCODING"] = "utf-8";
            cancellation.Token.ThrowIfCancellationRequested(); worker = Process.Start(start) ?? throw new IOException("이미지 처리 시작 실패");
            var errors = PumpLines(worker.StandardError, Log);
            await PumpLines(worker.StandardOutput, line =>
            {
                try
                {
                    using var doc = JsonDocument.Parse(line); var r = doc.RootElement;
                    if (r.GetProperty("type").GetString() == "result") { output = r.GetProperty("outputPath").GetString(); if (r.TryGetProperty("metadataPath", out var m)) { metadata = m.GetString(); Log("시트 좌표 JSON: " + metadata); } }
                    else if (r.TryGetProperty("message", out var m)) error = m.GetString();
                }
                catch (Exception ex) when (ex is JsonException or KeyNotFoundException) { Log(line); }
            });
            await worker.WaitForExitAsync(); await errors;
            if (cancellation.IsCancellationRequested) { Status.Text = "중지됨"; return; }
            if (worker.ExitCode != 0 || output is null || !File.Exists(output)) throw new IOException(error ?? "이미지 처리 실패");
            LoadImage(output); currentMetadata = metadata; Status.Text = "완료 · 원본은 보존되었습니다. PNG 내보내기로 저장하세요.";
        }
        catch (OperationCanceledException) { Status.Text = "중지됨"; }
        catch (Exception ex) { Fail(ex.Message); }
        finally
        {
            try { if (worker is { HasExited: false }) worker.Kill(true); } catch (Exception ex) { Log(ex.Message); }
            worker?.Dispose(); worker = null; cancellation?.Dispose(); cancellation = null; SetBusy(false);
            if (closeAfterCancel) Close();
        }
    }
}
public sealed class StudioSettings
{
    public string StylePrompt { get; set; } = "";
    public string StyleNegativePrompt { get; set; } = "";
    public string Prompt { get; set; } = "";
    public string Edit { get; set; } = "";
    public string NegativePrompt { get; set; } = "blurry, photo, text, watermark";
    public string Category { get; set; } = "character";
    public string Steps { get; set; } = "20";
    public string Seed { get; set; } = "-1";
    public int SizeIndex { get; set; } = 1;
    public bool Transparent { get; set; } = true;
    public bool ProtectInterior { get; set; } = true;
    public double Strength { get; set; } = .6;
    public string? ReferenceImage { get; set; }
}
