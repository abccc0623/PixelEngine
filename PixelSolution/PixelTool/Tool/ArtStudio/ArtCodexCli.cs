using System.Diagnostics;
using System.IO;
using System.Net.Http;
using System.Text;

namespace PixelTool;

internal static class ArtCodexCli
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

    private static async Task<(int Code, bool ChatLogin)> Run(ProcessStartInfo info, string? input, CancellationToken token)
    {
        token.ThrowIfCancellationRequested();
        using var process = Process.Start(info) ?? throw new IOException("Codex CLI 실행 실패");
        bool login = false;
        async Task Drain(StreamReader reader)
        {
            while (await reader.ReadLineAsync(token) is { } line)
                if (line.Contains("Logged in using ChatGPT", StringComparison.OrdinalIgnoreCase)) login = true;
        }
        using var cancel = token.Register(() => { try { process.Kill(true); } catch (InvalidOperationException) { } catch (System.ComponentModel.Win32Exception) { } });
        var stdout = Drain(process.StandardOutput);
        var stderr = Drain(process.StandardError);
        try
        {
            if (input != null) await process.StandardInput.WriteAsync(input.AsMemory(), token);
            process.StandardInput.Close();
            await Task.WhenAll(stdout, stderr, process.WaitForExitAsync(token));
            token.ThrowIfCancellationRequested();
            return (process.ExitCode, login);
        }
        finally { if (!process.HasExited) { try { process.Kill(true); } catch (InvalidOperationException) { } } }
    }

    internal static async Task CheckAsync(CancellationToken token)
    {
        var status = await Run(Command(Path.GetTempPath(), "login", "status"), null, token);
        if (status.Code != 0 || !status.ChatLogin)
            throw new InvalidOperationException("Codex CLI에서 ChatGPT 로그인이 필요합니다. 터미널에서 codex login 후 다시 확인하세요.");
    }

    internal static async Task<byte[]> GenerateAsync(byte[] reference, string prompt, CancellationToken token)
    {
        await CheckAsync(token);
        var dir = Directory.CreateTempSubdirectory("PixelTool-Art-").FullName;
        string source = Path.Combine(dir, "reference.png"), target = Path.Combine(dir, "result.png");
        try
        {
            await File.WriteAllBytesAsync(source, reference, token);
            var command = Command(dir, "exec", "--ephemeral", "--skip-git-repo-check", "--sandbox", "workspace-write",
                "-C", dir, "--image", source, "-");
            var run = await Run(command, "Use the built-in image_generation tool to create one PNG based on the attached reference and the art brief below. " +
                "Copy the generated image to result.png in this working directory. Do not draw using code, inspect projects, or install software. If generation fails, do not create a substitute.\nArt brief:\n" + prompt, token);
            var file = new FileInfo(target);
            if (run.Code != 0 || !file.Exists || file.Length is < 8 or > 33554432 || file.Attributes.HasFlag(FileAttributes.ReparsePoint))
                throw new IOException($"Codex CLI가 결과 PNG를 반환하지 않았습니다 (종료 코드 {run.Code}). 로그인·사용량·이미지 생성 지원 여부를 확인하세요.");
            var bytes = await File.ReadAllBytesAsync(target, token);
            if (!bytes.AsSpan(0, 8).SequenceEqual(new byte[] {137,80,78,71,13,10,26,10})) throw new IOException("결과가 PNG 파일이 아닙니다.");
            return bytes;
        }
        finally
        {
            try { File.Delete(source); File.Delete(target); Directory.Delete(dir, false); } catch (IOException) { }
        }
    }
}
