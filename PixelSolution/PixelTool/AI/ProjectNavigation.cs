using System.IO;
using System.Text.Json;

namespace PixelTool;

internal sealed class ProjectNavigation(string project)
{
    private readonly ProjectFileAccess _files = new(project);

    internal string Search(string query, string path, string mode, string extensions, int limit, CancellationToken token = default)
        => new ProjectFileSearch(_files.Root).Search(query, path, mode, extensions, limit, token);

    internal string ListDirectory(string path, int limit)
    {
        if (limit is < 1 or > 50) throw new ArgumentOutOfRangeException(nameof(limit));
        string fullPath = _files.Resolve(string.IsNullOrWhiteSpace(path) ? "Asset" : path);
        if (File.Exists(fullPath))
        {
            string relative = Path.GetRelativePath(_files.Root, fullPath).Replace('\\', '/');
            return JsonSerializer.Serialize(new { scope = relative, mode = "listing",
                entries = new[] { new { path = relative, kind = "file" } }, truncated = false });
        }
        if (!Directory.Exists(fullPath)) throw new DirectoryNotFoundException("프로젝트 폴더가 없습니다.");
        var entries = Directory.EnumerateFileSystemEntries(fullPath)
            .Where(entry => !ProjectFileAccess.Blocked(Path.GetFileName(entry)) &&
                (File.GetAttributes(entry) & FileAttributes.ReparsePoint) == 0)
            .Take(limit + 1).Select(entry => new
            {
                path = Path.GetRelativePath(_files.Root, entry).Replace('\\', '/'),
                kind = Directory.Exists(entry) ? "folder" : "file"
            }).ToArray();
        return JsonSerializer.Serialize(new { scope = Path.GetRelativePath(_files.Root, fullPath).Replace('\\', '/'),
            mode = "listing", entries = entries.Take(limit), truncated = entries.Length > limit });
    }

    internal string SearchFileContent(string path, string query, int limit, CancellationToken token = default)
    {
        if (!File.Exists(_files.Resolve(path))) throw new FileNotFoundException("검색할 프로젝트 파일이 없습니다.");
        return new ProjectFileSearch(_files.Root).Search(query, path, "content", "", limit, token);
    }
}
