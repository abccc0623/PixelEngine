using System.Diagnostics;
using System.IO;

namespace PixelTool;

internal static class PixelArtStudioLauncher
{
    internal static string ResolveExecutable(string baseDirectory)
    {
        // Installed packages isolate the self-contained app; solution builds share bin/<configuration>.
        string[] candidates = { Path.Combine(baseDirectory, "PixelArtStudio", "PixelArtStudio.exe"), Path.Combine(baseDirectory, "PixelArtStudio.exe") };
        return candidates.FirstOrDefault(File.Exists) ?? throw new FileNotFoundException(
            "Pixel Art Studio 실행 파일을 찾을 수 없습니다. 솔루션을 다시 빌드하거나 최신 PixelTool 설치 패키지로 다시 설치하세요.\n확인한 경로:\n" + string.Join("\n", candidates));
    }

    internal static ProcessStartInfo CreateStartInfo(string baseDirectory)
    {
        string executable = ResolveExecutable(baseDirectory);
        var info = new ProcessStartInfo(executable) { UseShellExecute = false, WorkingDirectory = Path.GetDirectoryName(executable)! };
        info.ArgumentList.Add("--mode");
        info.ArgumentList.Add("ui");
        return info;
    }

    internal static void Launch(string baseDirectory)
    {
        using var process = Process.Start(CreateStartInfo(baseDirectory)) ?? throw new IOException("Pixel Art Studio 프로세스를 시작하지 못했습니다.");
    }
}
