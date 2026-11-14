using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.Json;

namespace PixelArtStudio;

internal static class BackgroundRunner
{
    internal static string? Argument(string[] args, string key)
    {
        int index = Array.IndexOf(args, key);
        return index >= 0 && index + 1 < args.Length && !args[index + 1].StartsWith("--") ? args[index + 1] : null;
    }

    internal static async Task<int> RunAsync(string[] args, string mode)
    {
        string? resultPath = Argument(args, "--result");
        Process? process = null;
        var errors = new Queue<string>();
        JsonElement? generated = null;
        string? backendError = null;
        string state = "failed";
        string? error = null;
        int exitCode = 1;
        string? logPath = null;
        StreamWriter? log = null;
        Action stop = () => { try { if (process is { HasExited: false }) process.Kill(true); } catch (Exception) { } };
        EventHandler onExit = (_, _) => stop();
        AppDomain.CurrentDomain.ProcessExit += onExit;
        try
        {
            if (mode != "background") throw new ArgumentException("--mode must be ui or background.");
            if (string.IsNullOrWhiteSpace(resultPath)) throw new ArgumentException("Background mode requires --result <result.json>.");
            resultPath = Path.GetFullPath(resultPath);
            string requestPath = Path.GetFullPath(Argument(args, "--request") ?? throw new ArgumentException("Background mode requires --request <request.json>."));
            if (requestPath.Equals(resultPath, StringComparison.OrdinalIgnoreCase)) { resultPath = null; throw new ArgumentException("Request and result files must differ."); }
            string? cancelPath = Argument(args, "--cancel-file");
            if (cancelPath is not null) cancelPath = Path.GetFullPath(cancelPath);
            int timeoutSeconds = 600;
            if (Argument(args, "--timeout-seconds") is { } timeout && (!int.TryParse(timeout, out timeoutSeconds) || timeoutSeconds < 1 || timeoutSeconds > 86400)) throw new ArgumentException("Timeout must be 1..86400 seconds.");
            using var doc = JsonDocument.Parse(await File.ReadAllTextAsync(requestPath));
            var request = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(doc.RootElement.GetRawText()) ?? throw new ArgumentException("Request must be an object.");
            string root = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "PixelArtStudio");
            if (!request.TryGetValue("outputPath", out var output) || string.IsNullOrWhiteSpace(output.GetString()))
                request["outputPath"] = JsonSerializer.SerializeToElement(Path.Combine(root, "outputs", Guid.NewGuid().ToString("N"), "sprite.png"));
            string outputPath = Path.GetFullPath(request["outputPath"].GetString()!);
            if (outputPath.Equals(requestPath, StringComparison.OrdinalIgnoreCase) || outputPath.Equals(resultPath, StringComparison.OrdinalIgnoreCase)) { resultPath = null; throw new ArgumentException("Image, request and result paths must differ."); }
            Directory.CreateDirectory(Path.GetDirectoryName(resultPath)!);
            logPath = resultPath + ".log";
            log = new StreamWriter(logPath, false, new UTF8Encoding(false)) { AutoFlush = true };
            string runDir = Path.Combine(root, "jobs", Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(runDir);
            string normalizedRequest = Path.Combine(runDir, "request.json");
            await File.WriteAllTextAsync(normalizedRequest, JsonSerializer.Serialize(request));
            if (cancelPath is not null && File.Exists(cancelPath)) { state = "cancelled"; exitCode = 130; }
            else
            {
                string python = Argument(args, "--python") ?? Path.Combine(root, "venv", "Scripts", "python.exe");
                string script = Path.Combine(AppContext.BaseDirectory, "Backend", "generate.py");
                if (!File.Exists(python) || !File.Exists(script)) throw new IOException("Python runtime or backend is missing. Run Backend/setup.ps1.");
                var start = new ProcessStartInfo(python) { UseShellExecute = false, CreateNoWindow = true, RedirectStandardOutput = true, RedirectStandardError = true, StandardOutputEncoding = Encoding.UTF8, StandardErrorEncoding = Encoding.UTF8 };
                start.ArgumentList.Add(script); start.ArgumentList.Add("--request"); start.ArgumentList.Add(normalizedRequest);
                start.Environment["PYTHONIOENCODING"] = "utf-8"; start.Environment["HF_HOME"] = Path.Combine(root, "models");
                process = Process.Start(start) ?? throw new IOException("Could not start local image backend.");
                async Task Pump(StreamReader reader, bool stdout)
                {
                    while (await reader.ReadLineAsync() is { } line)
                    {
                        // Both pumps resume on the application dispatcher, so writes are serialized.
                        log.WriteLine(line);
                        if (!stdout) { errors.Enqueue(line); while (errors.Count > 8) errors.Dequeue(); continue; }
                        try
                        {
                            using var item = JsonDocument.Parse(line);
                            if (!item.RootElement.TryGetProperty("type", out var type)) continue;
                            if (type.GetString() == "result") generated = item.RootElement.Clone();
                            if (type.GetString() == "error") backendError = item.RootElement.GetProperty("message").GetString();
                        }
                        catch (JsonException) { }
                    }
                }
                Task stdout = Pump(process.StandardOutput, true), stderr = Pump(process.StandardError, false);
                var timer = Stopwatch.StartNew();
                bool cancelled = false, timedOut = false;
                while (!process.HasExited)
                {
                    cancelled = cancelPath is not null && File.Exists(cancelPath);
                    timedOut = timer.Elapsed.TotalSeconds >= timeoutSeconds;
                    if (cancelled || timedOut) { stop(); break; }
                    await Task.Delay(100);
                }
                await process.WaitForExitAsync(); await Task.WhenAll(stdout, stderr);
                if (cancelled) { state = "cancelled"; exitCode = 130; }
                else if (timedOut) { error = "Generation timed out."; exitCode = 124; }
                else if (process.ExitCode != 0 || generated is null) { error = backendError ?? $"Backend exited with code {process.ExitCode}. " + string.Join("\n", errors); }
                else if (!File.Exists(generated.Value.GetProperty("outputPath").GetString())) { error = "Backend did not produce its reported PNG."; }
                else { state = "completed"; exitCode = 0; }
            }
        }
        catch (ArgumentException ex) { error = ex.Message; exitCode = 2; }
        catch (JsonException ex) { error = ex.Message; exitCode = 2; }
        catch (Exception ex) { error = ex.Message; }
        finally { stop(); process?.Dispose(); log?.Dispose(); AppDomain.CurrentDomain.ProcessExit -= onExit; }
        if (!string.IsNullOrWhiteSpace(resultPath))
        {
            try
            {
                resultPath = Path.GetFullPath(resultPath);
                Directory.CreateDirectory(Path.GetDirectoryName(resultPath)!);
                string temp = resultPath + ".tmp-" + Guid.NewGuid().ToString("N");
                var result = new { status = state, exitCode, error, logPath, outputPath = state == "completed" ? generated?.GetProperty("outputPath").GetString() : null, generation = generated };
                await File.WriteAllTextAsync(temp, JsonSerializer.Serialize(result, new JsonSerializerOptions { WriteIndented = true }));
                File.Move(temp, resultPath, true);
            }
            catch { return 1; }
        }
        return exitCode;
    }
}
