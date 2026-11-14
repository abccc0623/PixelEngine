using Microsoft.Win32;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;

namespace PixelArtStudio;

public partial class MainWindow : Window
{
    private readonly string dataRoot = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "PixelArtStudio");
    private Process? worker;
    private CancellationTokenSource? cancellation;
    private string? currentImage;
    private string? resultPath;
    private bool busy;
    private bool closeAfterCancel;
    private static readonly JsonSerializerOptions JsonOptions = new() { PropertyNamingPolicy = JsonNamingPolicy.CamelCase, WriteIndented = true };

    public MainWindow()
    {
        InitializeComponent();
        PythonBox.Text = Path.Combine(dataRoot, "venv", "Scripts", "python.exe");
        Loaded += async (_, _) =>
        {
            try
            {
                string[] args = Environment.GetCommandLineArgs();
                string? Value(string key) { int i = Array.IndexOf(args, key); return i >= 0 && i + 1 < args.Length ? args[i + 1] : null; }
                resultPath = Value("--result");
                if (Value("--request") is { } request)
                {
                    var r = JsonSerializer.Deserialize<GenerationRequest>(await File.ReadAllTextAsync(request), new JsonSerializerOptions { PropertyNameCaseInsensitive = true }) ?? throw new InvalidDataException("요청 JSON이 비어 있습니다.");
                    StylePromptBox.Text = r.StylePrompt; StyleNegativeBox.Text = r.StyleNegativePrompt; PromptBox.Text = r.Prompt; NegativeBox.Text = r.NegativePrompt; CategoryBox.SelectedIndex = r.Category switch { "background" => 1, "tile" => 2, _ => 0 };
                    StepsBox.Text = r.Steps.ToString(); SeedBox.Text = r.Seed.ToString();
                    TransparentBox.IsChecked = r.Transparent; ProtectInteriorBox.IsChecked = r.ProtectInterior; StrengthSlider.Value = r.Strength;
                    SizeBox.SelectedIndex = r.Width switch { 32 => 0, 64 => 1, 128 => 2, _ => throw new InvalidDataException("크기는 32, 64, 128만 지원합니다.") };
                    if (r.Height != r.Width) throw new InvalidDataException("현재 앱은 정사각형 출력만 지원합니다.");
                    if (!string.IsNullOrEmpty(r.ReferenceImage)) LoadImage(r.ReferenceImage);
                    if (args.Contains("--generate")) await GenerateAsync(!string.IsNullOrEmpty(r.ReferenceImage));
                }
            }
            catch (Exception ex) { await RejectAsync(ex.Message); }
        };
    }

    private void LoadImage(string path)
    {
        var bitmap = new BitmapImage();
        bitmap.BeginInit(); bitmap.CacheOption = BitmapCacheOption.OnLoad; bitmap.UriSource = new Uri(Path.GetFullPath(path)); bitmap.EndInit(); bitmap.Freeze();
        Preview.Source = bitmap; currentImage = Path.GetFullPath(path); sourceImage = null; processedImage = null; currentMetadata = null;
        EmptyHint.Visibility = Visibility.Collapsed;
        ImageInfo.Text = $"{bitmap.PixelWidth} × {bitmap.PixelHeight}";
        EditButton.IsEnabled = ExportButton.IsEnabled = !busy;
    }
    private void Open_Click(object sender, RoutedEventArgs e)
    {
        var dialog = new OpenFileDialog { Filter = "PNG 이미지|*.png", CheckFileExists = true };
        if (dialog.ShowDialog(this) == true) try { LoadImage(dialog.FileName); } catch (Exception ex) { Fail(ex.Message); }
    }
    private void Export_Click(object sender, RoutedEventArgs e)
    {
        if (currentImage is null) return;
        var dialog = new SaveFileDialog { Filter = "PNG 이미지|*.png", FileName = "sprite.png", AddExtension = true, OverwritePrompt = true };
        if (dialog.ShowDialog(this) == true)
        {
            try { if (!Path.GetFullPath(dialog.FileName).Equals(currentImage, StringComparison.OrdinalIgnoreCase)) File.Copy(currentImage, dialog.FileName, true); Log("저장: " + dialog.FileName); Status.Text = "PNG 저장 완료"; }
            catch (Exception ex) { Fail(ex.Message); }
        }
    }
    private async void Generate_Click(object sender, RoutedEventArgs e) { if (busy) return; ClearWorkItemForDirectGeneration(); await GenerateAsync(false); }
    private async void Edit_Click(object sender, RoutedEventArgs e) { if (busy) return; ClearWorkItemForDirectGeneration(); await GenerateAsync(true); }

    private async Task GenerateAsync(bool edit, string? referenceOverride = null)
    {
        if (busy) return;
        if (!ValidateWorkItemPrompt(edit)) return;
        if (string.IsNullOrWhiteSpace(PromptBox.Text)) { await RejectAsync("이미지 설명을 입력하세요."); return; }
        if (edit && referenceOverride is null && currentImage is null) { await RejectAsync("수정할 이미지를 먼저 열어 주세요."); return; }
        if (!int.TryParse(StepsBox.Text, out int steps) || steps < 4 || steps > 50 || !long.TryParse(SeedBox.Text, out long seed) || seed < -1 || seed > uint.MaxValue) { await RejectAsync("단계는 4~50, 시드는 -1~4294967295 범위로 입력하세요."); return; }
        string python = PythonBox.Text.Trim();
        string script = Path.Combine(AppContext.BaseDirectory, "Backend", "generate.py");
        if (!File.Exists(python) || !File.Exists(script)) { await RejectAsync("로컬 생성 환경이 없습니다. README의 Backend/setup.ps1 설치를 먼저 실행하세요."); return; }
        SetBusy(true); cancellation = new CancellationTokenSource(); Progress.Value = 0; LogBox.Clear();
        string runDir = Path.Combine(dataRoot, "outputs", DateTime.Now.ToString("yyyyMMdd-HHmmss") + "-" + Guid.NewGuid().ToString("N")[..8]);
        string? generatedSource = null; string? generated = null; string? generatedWarning = null; string? backendError = null; long actualSeed = seed;
        try
        {
            Directory.CreateDirectory(runDir);
            if (appliedWorkItem is not null) await File.WriteAllTextAsync(Path.Combine(runDir, "workitem.json"), JsonSerializer.Serialize(appliedWorkItem, JsonOptions));
            int size = int.Parse((string)((ComboBoxItem)SizeBox.SelectedItem).Tag);
            var request = new GenerationRequest { Prompt = edit && !string.IsNullOrWhiteSpace(EditBox.Text) ? EditBox.Text.Trim() : PromptBox.Text.Trim(), StylePrompt = StylePromptBox.Text, StyleNegativePrompt = StyleNegativeBox.Text, Category = SelectedCategory, NegativePrompt = NegativeBox.Text.Trim(), Width = size, Height = size, Steps = steps, Seed = seed, Transparent = TransparentBox.IsChecked == true, ProtectInterior = ProtectInteriorBox.IsChecked == true, ReferenceImage = edit ? referenceOverride ?? currentImage : null, Strength = StrengthSlider.Value, OutputPath = Path.Combine(runDir, "sprite.png") };
            string requestPath = Path.Combine(runDir, "request.json");
            await File.WriteAllTextAsync(requestPath, JsonSerializer.Serialize(request, JsonOptions));
            var start = new ProcessStartInfo(python) { UseShellExecute = false, CreateNoWindow = true, RedirectStandardOutput = true, RedirectStandardError = true, StandardOutputEncoding = Encoding.UTF8, StandardErrorEncoding = Encoding.UTF8 };
            start.ArgumentList.Add(script); start.ArgumentList.Add("--request"); start.ArgumentList.Add(requestPath);
            start.Environment["PYTHONIOENCODING"] = "utf-8"; start.Environment["HF_HOME"] = Path.Combine(dataRoot, "models");
            cancellation.Token.ThrowIfCancellationRequested();
            worker = Process.Start(start) ?? throw new IOException("생성 프로세스를 시작하지 못했습니다.");
            Status.Text = "로컬 모델 준비 중";
            var errorTask = PumpLines(worker.StandardError, line => Log(line));
            await PumpLines(worker.StandardOutput, line =>
            {
                try
                {
                    using var doc = JsonDocument.Parse(line); var root = doc.RootElement;
                    string? type = root.GetProperty("type").GetString();
                    if (type == "progress")
                    {
                        if (root.TryGetProperty("message", out var m)) { Status.Text = m.GetString(); Log(m.GetString() ?? ""); }
                        if (root.TryGetProperty("percent", out var p)) Progress.Value = Math.Clamp(p.GetDouble(), 0, 100);
                    }
                    else if (type == "result") { generated = root.GetProperty("outputPath").GetString(); if (root.TryGetProperty("seed", out var s)) actualSeed = s.GetInt64(); if (root.TryGetProperty("warning", out var w)) generatedWarning = w.GetString(); if (root.TryGetProperty("sourcePath", out var source)) { generatedSource = source.GetString(); Log("배경 제거 전 원본: " + generatedSource); } }
                    else if (type == "error") { backendError = root.GetProperty("message").GetString(); Log(backendError ?? "생성 오류"); }
                }
                catch (JsonException) { Log(line); }
                catch (KeyNotFoundException) { Log(line); }
            });
            await worker.WaitForExitAsync(); await errorTask;
            if (cancellation.IsCancellationRequested) { Status.Text = "중지됨 · 기존 이미지는 유지됩니다."; await SaveResult("cancelled", null, null); }
            else if (worker.ExitCode != 0 || string.IsNullOrWhiteSpace(generated) || !File.Exists(generated)) throw new IOException(backendError ?? $"생성 실패 (종료 코드 {worker.ExitCode}). 아래 로그를 확인하세요.");
            else { LoadImage(generated); sourceImage = generatedSource; processedImage = generated; SeedBox.Text = actualSeed.ToString(); request.Seed = actualSeed; await File.WriteAllTextAsync(Path.Combine(runDir, "request.json"), JsonSerializer.Serialize(request, JsonOptions)); var savedSettings = CaptureSettings(); savedSettings.ReferenceImage = request.ReferenceImage; await File.WriteAllTextAsync(Path.Combine(runDir, "settings.json"), JsonSerializer.Serialize(savedSettings, JsonOptions)); Progress.Value = 100; Status.Text = string.IsNullOrEmpty(generatedWarning) ? $"완료 · 시드 {actualSeed}" : "완료 · 주의: " + generatedWarning; await SaveResult("completed", generated, null); }
        }
        catch (OperationCanceledException) { Status.Text = "중지됨"; try { await SaveResult("cancelled", null, null); } catch (Exception ex) { Log(ex.Message); } }
        catch (Exception ex) { Fail(ex.Message); try { await SaveResult("failed", null, ex.Message); } catch (Exception resultError) { Log("결과 파일 저장 실패: " + resultError.Message); } }
        finally
        {
            try { if (worker is { HasExited: false }) worker.Kill(true); } catch (InvalidOperationException) { } catch (Win32Exception ex) { Log("프로세스 종료 실패: " + ex.Message); }
            worker?.Dispose(); worker = null; cancellation?.Dispose(); cancellation = null; SetBusy(false);
            if (closeAfterCancel) Close();
        }
    }
    private async Task PumpLines(StreamReader reader, Action<string> callback)
    {
        while (await reader.ReadLineAsync() is { } line) await Dispatcher.InvokeAsync(() => callback(line));
    }
    private async Task SaveResult(string state, string? output, string? error)
    {
        if (resultPath is null) return;
        string full = Path.GetFullPath(resultPath); Directory.CreateDirectory(Path.GetDirectoryName(full)!);
        string temp = full + ".tmp-" + Guid.NewGuid().ToString("N");
        await File.WriteAllTextAsync(temp, JsonSerializer.Serialize(new { status = state, outputPath = output, error }, JsonOptions));
        File.Move(temp, full, true);
    }
    private async Task RejectAsync(string message)
    {
        Fail(message);
        try { await SaveResult("failed", null, message); } catch (Exception ex) { Log("결과 파일 저장 실패: " + ex.Message); }
    }
    private void SetBusy(bool value)
    {
        WorkItemGenerateButton.IsEnabled = SharedStylePanel.IsEnabled = WorkItemPanel.IsEnabled = FeaturePanel.IsEnabled = ImageToolsPanel.IsEnabled = !value; busy = value; GenerateButton.IsEnabled = OpenButton.IsEnabled = !value;
        EditButton.IsEnabled = ExportButton.IsEnabled = !value && currentImage is not null;
        CancelButton.IsEnabled = value; StrengthSlider.IsEnabled = !value; PromptBox.IsEnabled = EditBox.IsEnabled = PythonBox.IsEnabled = SizeBox.IsEnabled = StepsBox.IsEnabled = SeedBox.IsEnabled = TransparentBox.IsEnabled = ProtectInteriorBox.IsEnabled = !value;
    }
    private void Log(string message)
    {
        LogBox.AppendText($"[{DateTime.Now:HH:mm:ss}] {message}\n");
        if (LogBox.Text.Length > 16000) LogBox.Text = LogBox.Text[^12000..];
        LogBox.ScrollToEnd();
    }
    private void Fail(string message) { Status.Text = "실패 · " + message; Log(message); }
    private void Cancel_Click(object sender, RoutedEventArgs e)
    {
        cancellation?.Cancel(); CancelButton.IsEnabled = false; Status.Text = "중지 중";
        try { if (worker is { HasExited: false }) worker.Kill(true); } catch (InvalidOperationException) { } catch (Win32Exception ex) { Fail("프로세스 중지 실패: " + ex.Message); }
    }
    private void Window_Closing(object? sender, CancelEventArgs e)
    {
        if (!busy) return; e.Cancel = true; closeAfterCancel = true; Cancel_Click(this, new RoutedEventArgs());
    }
}
public sealed class GenerationRequest
{
    public string StylePrompt { get; set; } = "";
    public string StyleNegativePrompt { get; set; } = "";
    public string Category { get; set; } = "character";
    public string Prompt { get; set; } = "";
    public string NegativePrompt { get; set; } = "blurry, photo, text, watermark";
    public int Width { get; set; } = 64;
    public int Height { get; set; } = 64;
    public int Steps { get; set; } = 20;
    public long Seed { get; set; } = -1;
    public string? ReferenceImage { get; set; }
    public double Strength { get; set; } = 0.6;
    public bool Transparent { get; set; } = true;
    public bool ProtectInterior { get; set; } = true;
    public string OutputPath { get; set; } = "";
}
