using System.Diagnostics;
using System.IO;
using System.Net.Http;
using System.Text;

namespace PixelTool;

internal static class CodexCli
{
    internal const string InstallGuide = "https://developers.openai.com/codex/cli/";
    private const string InstallScript = "https://chatgpt.com/codex/install.ps1";

    internal static async Task InstallAsync(CancellationToken token)
    {
        var script = Path.Combine(Path.GetTempPath(), $"PixelTool-CodexInstall-{Guid.NewGuid():N}.ps1");
        try
        {
            using var client = new HttpClient { Timeout = TimeSpan.FromMinutes(2) };
            var bytes = await client.GetByteArrayAsync(InstallScript, token);
            await File.WriteAllBytesAsync(script, bytes, token);
            var info = new ProcessStartInfo("powershell.exe")
            {
                UseShellExecute = false, CreateNoWindow = true,
                RedirectStandardOutput = true, RedirectStandardError = true
            };
            foreach (var arg in new[] { "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", script })
                info.ArgumentList.Add(arg);
            using var process = Process.Start(info) ?? throw new IOException("Codex CLI installer could not start.");
            using var cancel = token.Register(() => { try { process.Kill(true); } catch { } });
            var stdout = process.StandardOutput.ReadToEndAsync(token);
            var stderr = process.StandardError.ReadToEndAsync(token);
            await Task.WhenAll(stdout, stderr, process.WaitForExitAsync(token));
            if (process.ExitCode != 0)
                throw new IOException($"Codex CLI installation failed ({process.ExitCode}): {await stderr}");
            var paths = new[] { Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.Process),
                Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.User),
                Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.Machine) };
            Environment.SetEnvironmentVariable("PATH", string.Join(';', paths.Where(p => !string.IsNullOrWhiteSpace(p))));
        }
        finally { try { File.Delete(script); } catch (IOException) { } }
    }

    internal static ProcessStartInfo Command(string work, params string[] args)
    {
        var paths = new[] { Environment.GetEnvironmentVariable("PATH"),
            Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.User),
            Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.Machine) };
        var dirs = string.Join(';', paths).Split(';')
            .Select(p => p.Trim().Trim('"')).Where(Path.IsPathFullyQualified).ToArray();
        string? Find(string file) => dirs.Select(p => Path.Combine(p, file)).FirstOrDefault(File.Exists);
        var exe = Find("codex.exe");
        var info = new ProcessStartInfo { WorkingDirectory = work, UseShellExecute = false, CreateNoWindow = true,
            RedirectStandardInput = true, StandardInputEncoding = new UTF8Encoding(false),
            RedirectStandardOutput = true, RedirectStandardError = true };
        if (exe != null) info.FileName = exe;
        else if (Find("node.exe") is { } node && Find(@"node_modules\@openai\codex\bin\codex.js") is { } script)
        {
            info.FileName = node;
            info.ArgumentList.Add(script);
        }
        else throw new FileNotFoundException("Codex CLI를 찾지 못했습니다. 공식 설치 안내에 따라 설치하고 PixelTool을 다시 실행하세요.");
        info.Environment.Remove("OPENAI_API_KEY");
        info.Environment.Remove("CODEX_API_KEY");
        foreach (var arg in args) info.ArgumentList.Add(arg);
        return info;
    }

}
