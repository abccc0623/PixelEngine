using System.Diagnostics;
using System.IO;
using System.Text;

namespace PixelTool;

// File originals remain authoritative. No LLM, database, or project-to-project knowledge sharing.
internal sealed class WikiSearchService
{
    internal static readonly WikiSearchService Shared = new(Path.Combine(AppContext.BaseDirectory, "LLMWiki"));
    internal sealed record Hit(string Document, string Title, int Line, string Snippet, int Score);
    internal sealed record Result(Hit[] Matches, double ElapsedMs, int Documents, bool Truncated, string[] Warnings);
    private sealed record Entry(long Size, DateTime Modified, string Text, string Title, string[] Aliases);
    private readonly string root;
    private readonly object gate = new();
    private Dictionary<string, Entry> cache = new(StringComparer.OrdinalIgnoreCase);
    private const int MaxBytes = 1024 * 1024, MaxFiles = 10000;
    private readonly string[] categories;
    internal WikiSearchService(string root, string[]? categories = null)
    {
        this.root = Path.TrimEndingDirectorySeparator(Path.GetFullPath(root));
        this.categories = categories ?? ["API", "Skills", "Rules", "Knowledge"];
    }

    internal Result Search(string query, int limit = 20, CancellationToken cancellation = default)
    {
        if (string.IsNullOrWhiteSpace(query) || query.Length > 200 || limit is < 1 or > 100)
            throw new ArgumentException("검색어는 1~200자, 결과 수는 1~100이어야 합니다.");
        var clock = Stopwatch.StartNew();
        Entry[] entries;
        string[] paths;
        var warnings = new List<string>();
        bool truncated = false;
        // Publish a complete new cache only. Cancellation never publishes a half-refreshed index.
        lock (gate)
        {
            cancellation.ThrowIfCancellationRequested();
            var next = new Dictionary<string, Entry>(StringComparer.OrdinalIgnoreCase);
            long bytes = 0;
            foreach (string path in Enumerate(cancellation, warnings))
            {
                cancellation.ThrowIfCancellationRequested();
                if (next.Count >= MaxFiles) { truncated = true; break; }
                try
                {
                    var info = new FileInfo(path);
                    if (info.Length > MaxBytes || bytes + info.Length > 64L * MaxBytes) { truncated = true; continue; }
                    bytes += info.Length;
                    if (!cache.TryGetValue(path, out var entry) || entry.Size != info.Length || entry.Modified != info.LastWriteTimeUtc)
                    {
                        string text = ReadBounded(path);
                        string[] lines = text.Split('\n');
                        string title = lines.FirstOrDefault(l => l.StartsWith("# ", StringComparison.Ordinal))?[2..].Trim()
                            ?? Path.GetFileNameWithoutExtension(path);
                        string[] aliases = lines.Take(40).Where(l => l.StartsWith("aliases:", StringComparison.OrdinalIgnoreCase))
                            .SelectMany(l => l[8..].Trim(' ', '[', ']', '\r').Split(',', StringSplitOptions.RemoveEmptyEntries))
                            .Select(a => a.Trim(' ', '\'', '"')).Where(a => a.Length > 0).ToArray();
                        entry = new(info.Length, info.LastWriteTimeUtc, text, title, aliases);
                    }
                    next.Add(path, entry);
                }
                catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
                { if (warnings.Count < 10) warnings.Add(Path.GetFileName(path) + ": 읽을 수 없습니다."); }
            }
            cancellation.ThrowIfCancellationRequested();
            cache = next;
            paths = next.Keys.ToArray(); entries = next.Values.ToArray();
        }
        var hits = new List<Hit>();
        string needle = query.Trim();
        string[] words = needle.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);
        for (int i = 0; i < paths.Length; i++)
        {
            cancellation.ThrowIfCancellationRequested();
            var e = entries[i];
            string name = Path.GetFileNameWithoutExtension(paths[i]);
            int offset = e.Text.IndexOf(needle, StringComparison.OrdinalIgnoreCase);
            bool Exact(string value) => string.Equals(value, needle, StringComparison.OrdinalIgnoreCase);
            int score = Exact(name) || Exact(e.Title) ? 100 : e.Aliases.Any(Exact) ? 90
                : name.Contains(needle, StringComparison.OrdinalIgnoreCase) || e.Title.Contains(needle, StringComparison.OrdinalIgnoreCase) ? 70
                : offset >= 0 ? 50 : 0;
            if (score == 0 && words.All(w => e.Text.Contains(w, StringComparison.OrdinalIgnoreCase)))
            { score = 30; offset = e.Text.IndexOf(words[0], StringComparison.OrdinalIgnoreCase); }
            if (score == 0) continue;
            offset = Math.Max(0, offset);
            int line = 1;
            for (int j = 0; j < offset; j++) if (e.Text[j] == '\n') line++;
            int start = Math.Max(0, offset - 40), end = Math.Min(e.Text.Length, start + 240);
            if (start > 0 && char.IsLowSurrogate(e.Text[start])) start++;
            if (end > start && end < e.Text.Length && char.IsHighSurrogate(e.Text[end - 1])) end--;
            hits.Add(new("wiki/" + Path.GetRelativePath(root, paths[i]).Replace('\\', '/'), e.Title, line,
                e.Text[start..end], score));
        }
        var ordered = hits.OrderByDescending(h => h.Score).ThenBy(h => h.Document, StringComparer.Ordinal).Take(limit).ToArray();
        return new(ordered, clock.Elapsed.TotalMilliseconds, entries.Length, truncated || hits.Count > limit, warnings.ToArray());
    }

    internal string Read(string document)
    {
        if (!document.StartsWith("wiki/", StringComparison.Ordinal)) throw new ArgumentException("Invalid wiki document.");
        string relative = document[5..];
        string[] parts = relative.Split('/', '\\');
        if (parts.Length < 2 || !categories.Contains(parts[0]) || parts.Any(p => p is "" or "." or ".." || p.Contains(':')))
            throw new ArgumentException("Invalid wiki document.");
        string path = Path.GetFullPath(Path.Combine(root, relative));
        if (!path.StartsWith(root + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase) || !Supported(path))
            throw new ArgumentException("Invalid wiki document.");
        string current = root;
        if (IsLink(current)) throw new IOException("Linked wiki roots are not supported.");
        foreach (var part in parts) { current = Path.Combine(current, part); if (IsLink(current)) throw new IOException("Linked wiki paths are not supported."); }
        return ReadBounded(path);
    }
    private static string ReadBounded(string path)
    {
        using var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete);
        if (stream.Length > MaxBytes) throw new IOException("Wiki document exceeds 1 MB.");
        using var reader = new StreamReader(stream, Encoding.UTF8, true);
        var buffer = new char[(int)Math.Min(stream.Length + 1, MaxBytes + 1)];
        int length = reader.ReadBlock(buffer, 0, buffer.Length);
        if (length > MaxBytes) throw new IOException("Wiki document exceeds read limit.");
        return new string(buffer, 0, length);
    }
    private static bool Supported(string path) => Path.GetExtension(path).ToLowerInvariant() is ".md" or ".lua" or ".txt";
    private static bool IsLink(string path) => (File.GetAttributes(path) & FileAttributes.ReparsePoint) != 0;
    private IEnumerable<string> Enumerate(CancellationToken token, List<string> warnings)
    {
        if (!Directory.Exists(root)) yield break;
        if (IsLink(root)) { warnings.Add("연결된 위키 폴더는 검색하지 않습니다."); yield break; }
        var pending = new Stack<string>(categories.Select(c => Path.Combine(root, c)).Where(Directory.Exists).Reverse());
        while (pending.TryPop(out var directory))
        {
            token.ThrowIfCancellationRequested();
            string[] files, folders;
            try
            {
                if (IsLink(directory)) continue;
                files = Directory.GetFiles(directory).OrderBy(p => p, StringComparer.Ordinal).ToArray();
                folders = Directory.GetDirectories(directory).OrderByDescending(p => p, StringComparer.Ordinal).ToArray();
            }
            catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
            { if (warnings.Count < 10) warnings.Add(Path.GetFileName(directory) + ": 폴더를 읽을 수 없습니다."); continue; }
            foreach (var folder in folders) pending.Push(folder);
            foreach (var file in files)
            {
                token.ThrowIfCancellationRequested();
                bool include;
                try { include = Supported(file) && !IsLink(file); }
                catch (IOException) { continue; }
                if (include) yield return file;
            }
        }
    }
}
