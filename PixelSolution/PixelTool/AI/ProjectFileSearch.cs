using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.Json;

namespace PixelTool;

// Deterministic local search. UI snapshots and AI/MCP searches use the same filename matching.
// The browser keeps its selected-folder scope; AI tools use the bounded recursive project scan.
internal sealed class ProjectFileSearch(string project)
{
    private readonly ProjectFileAccess _files = new(project);
    private static readonly UTF8Encoding Utf8 = new(false, true);

    internal const string ExtensionDescription = "Comma- or semicolon-separated file extensions, e.g. .lua,.pxm, lua;pxm, or *.lua;*.pxm. Filename mode always includes matching folders; extensions filter files only. Empty means supported text files; * alone includes all file names. Content mode accepts only supported text extensions, never binary types. At most 12 distinct extensions and 200 input characters.";

    private static (bool AllNames, HashSet<string> Filter) ParseExtensions(string? extensions, string mode)
    {
        ArgumentException Invalid(string reason) => new(
            $"확장자 입력 {JsonSerializer.Serialize(extensions?.Length > 200 ? extensions[..200] + "..." : extensions)}: {reason} " +
            ".lua,.pxm 또는 lua;pxm 또는 *.lua;*.pxm처럼 지정하세요. 빈 문자열은 지원 텍스트 형식이며, * 단독은 filename 모드에서만 가능합니다.",
            nameof(extensions));

        if (extensions?.Length > 200) throw Invalid("200자 이내로 지정하세요.");
        if (string.IsNullOrWhiteSpace(extensions)) return (false, ProjectFileAccess.TextExtensions);
        if (extensions.Trim() == "*")
        {
            if (mode == "filename") return (true, new HashSet<string>(StringComparer.OrdinalIgnoreCase));
            throw Invalid("본문 검색에는 *를 사용할 수 없습니다. 빈 문자열 또는 지원 텍스트 확장자를 지정하세요.");
        }

        var filter = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (string item in extensions.Split(new[] { ',', ';' }, StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            string extension = item.StartsWith("*.") ? item[1..] : item.StartsWith('.') ? item : "." + item;
            // Path.GetExtension matches one suffix, not paths, glob patterns or compound suffixes.
            if (extension.Length is < 2 or > 16 || extension.Skip(1).Any(c => !char.IsLetterOrDigit(c) && c is not ('_' or '-')))
                throw Invalid("경로, 복합 확장자와 임의 패턴은 지원하지 않습니다. 확장자는 점 뒤 1~15자의 문자·숫자·_·-로 지정하세요.");
            filter.Add(extension);
        }
        if (filter.Count is < 1 or > 12) throw Invalid("확장자를 1~12개 지정하세요.");
        if (mode == "content" && filter.Any(e => !ProjectFileAccess.TextExtensions.Contains(e)))
            throw Invalid("본문 검색은 지원 텍스트 형식만 가능합니다: " + string.Join(",", ProjectFileAccess.TextExtensions) +
                ". 이미지 등 다른 형식은 filename 모드로 찾으세요.");
        return (false, filter);
    }

    internal static bool MatchesFileName(string fileName, string query)
        => fileName.Contains(query, StringComparison.OrdinalIgnoreCase);

    // Filter the already-loaded browser snapshot without disk I/O, limits, or selection changes.
    internal static IEnumerable<T> FilterFileNames<T>(IEnumerable<T> files, Func<T, string> fileName, string? query)
    {
        string keyword = query?.Trim() ?? "";
        return keyword.Length == 0 ? files : files.Where(file => MatchesFileName(fileName(file), keyword));
    }

    internal string Search(string query, string path, string mode, string extensions, int limit, CancellationToken token = default)
    {
        if (string.IsNullOrWhiteSpace(query) || query.Length > 200) throw new ArgumentException("검색어는 1~200자여야 합니다.");
        if (mode is not ("filename" or "content")) throw new ArgumentException("검색 모드는 filename 또는 content입니다.");
        if (limit is < 1 or > 50) throw new ArgumentOutOfRangeException(nameof(limit), "결과 개수는 1~50입니다.");
        string root = _files.Resolve(string.IsNullOrWhiteSpace(path) ? "Asset" : path);
        string? exactFile = File.Exists(root) ? root : null;
        if (exactFile != null && mode != "content") throw new ArgumentException("파일 경로는 내용 검색에만 사용할 수 있습니다.");
        if (exactFile == null && !Directory.Exists(root)) throw new DirectoryNotFoundException("검색할 프로젝트 폴더 또는 파일이 없습니다.");
        string scope = Path.GetRelativePath(_files.Root, root).Replace('\\', '/');
        bool includeEngine = scope.Equals("Engine", StringComparison.OrdinalIgnoreCase) || scope.StartsWith("Engine/", StringComparison.OrdinalIgnoreCase);
        var (allNames, filter) = ParseExtensions(extensions, mode);
        if (exactFile != null && !filter.Contains(Path.GetExtension(exactFile)))
            throw new InvalidOperationException("지원하는 텍스트 파일 경로가 필요합니다.");

        var matches = new List<object>();
        var pending = new Stack<string>();
        pending.Push(exactFile == null ? root : Path.GetDirectoryName(exactFile)!);
        int visitedEntries = 0, searchedFiles = 0, searchedFolders = 0, skippedFiles = 0, outputCharacters = 0;
        long readBytes = 0;
        bool truncated = false;
        string? stopReason = null;
        var clock = Stopwatch.StartNew();
        string Result() => JsonSerializer.Serialize(new { scope, mode, matches, searchedFiles, searchedFolders, skippedFiles, truncated, stopReason,
            note = "검색 결과는 파일·폴더 위치 힌트입니다. 파일 수정 전 실제 문맥과 최신 해시를 확인하세요. 잘림/건너뛴 파일이 있으면 검색 범위를 좁히세요." });
        bool Add(string relative, string kind, int? line, int? column, string snippet, bool snippetTruncated)
        {
            if (outputCharacters + relative.Length + snippet.Length > 12000) { truncated = true; stopReason = "output_limit"; return false; }
            matches.Add(new { path = relative, kind, line, column, snippet, snippetTruncated });
            outputCharacters += relative.Length + snippet.Length;
            if (matches.Count < limit) return true;
            truncated = true; stopReason = "result_limit"; return false;
        }
        while (pending.Count > 0)
        {
            token.ThrowIfCancellationRequested();
            string directory = pending.Pop();
            try
            {
                _files.Resolve(Path.GetRelativePath(_files.Root, directory));
                foreach (string entry in exactFile == null ? Directory.EnumerateFileSystemEntries(directory) : new[] { exactFile })
                {
                    token.ThrowIfCancellationRequested();
                    if (++visitedEntries > 5000 || clock.Elapsed > TimeSpan.FromSeconds(3))
                    { truncated = true; stopReason = "scan_limit"; return Result(); }
                    string name = Path.GetFileName(entry);
                    if (ProjectFileAccess.Blocked(name)) continue;
                    try
                    {
                        if ((File.GetAttributes(entry) & FileAttributes.ReparsePoint) != 0) continue;
                        string relative = Path.GetRelativePath(_files.Root, entry).Replace('\\', '/');
                        string safePath = _files.Resolve(relative);
                        if (Directory.Exists(safePath))
                        {
                            if ((!includeEngine && relative.Equals("Engine", StringComparison.OrdinalIgnoreCase)) ||
                                name.Equals("bin", StringComparison.OrdinalIgnoreCase) || name.Equals("obj", StringComparison.OrdinalIgnoreCase) ||
                                name.Equals("node_modules", StringComparison.OrdinalIgnoreCase) || name.Equals("packages", StringComparison.OrdinalIgnoreCase)) continue;
                            searchedFolders++;
                            if (mode == "filename" && MatchesFileName(name, query) && !Add(relative, "folder", null, null, "", false)) return Result();
                            pending.Push(safePath);
                            continue;
                        }
                        if (!allNames && !filter.Contains(Path.GetExtension(safePath))) continue;
                        searchedFiles++;
                        if (mode == "filename")
                        {
                            if (MatchesFileName(name, query) && !Add(relative, "file", null, null, "", false)) return Result();
                            continue;
                        }
                        long length = new FileInfo(safePath).Length;
                        if (length > 256 * 1024) { skippedFiles++; continue; }
                        if (readBytes + length > 4 * 1024 * 1024) { truncated = true; stopReason = "byte_limit"; return Result(); }
                        byte[] bytes = ProjectFileAccess.ReadBounded(safePath);
                        readBytes += bytes.Length;
                        string text = Utf8.GetString(bytes).TrimStart('\uFEFF');
                        if (text.Contains('\0')) { skippedFiles++; continue; }
                        using var reader = new StringReader(text);
                        int lineNumber = 0;
                        while (reader.ReadLine() is { } line)
                        {
                            token.ThrowIfCancellationRequested();
                            lineNumber++;
                            for (int at = line.IndexOf(query, StringComparison.OrdinalIgnoreCase); at >= 0;
                                at = line.IndexOf(query, at + 1, StringComparison.OrdinalIgnoreCase))
                            {
                                int start = Math.Max(0, at - 80), count = Math.Min(240, line.Length - start);
                                if (!Add(relative, "file", lineNumber, at + 1, line.Substring(start, count), start > 0 || count < line.Length)) return Result();
                            }
                        }
                    }
                    catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidOperationException or DecoderFallbackException)
                    { skippedFiles++; }
                }
            }
            catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidOperationException)
            { skippedFiles++; }
        }
        return Result();
    }
}
