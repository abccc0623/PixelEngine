using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace PixelTool;

// Work-item definitions belong to the open project, never to the installed wiki.
internal sealed class ProjectWorkItemWiki(string project)
{
    internal const string Folder = "Editor/WorkItems";
    private readonly ProjectFileAccess files = new(project);
    private readonly WikiSearchService search = new(Path.Combine(project, "Editor"), ["WorkItems"]);

    internal (string Id, string Path) Create(string title, string content, string precautions, string[] keywords, string[] checks)
    {
        static string Clean(string value, int max, string field)
        {
            value = value.Trim();
            if (value.Length is 0 or > 12000 || value.Length > max || value.Any(c => char.IsControl(c) && c is not ('\n' or '\r' or '\t')))
                throw new ArgumentException($"Invalid {field}.");
            return value;
        }
        title = Clean(title, 200, "title").Replace('\r', ' ').Replace('\n', ' ');
        string fileTitle = new string(title.Select(c => Path.GetInvalidFileNameChars().Contains(c) ? '_' : c).ToArray())
            .Trim(' ', '.', '_');
        if (fileTitle.Length > 80) fileTitle = fileTitle[..80].TrimEnd(' ', '.', '_');
        if (fileTitle.Length == 0) fileTitle = "일감";
        content = Clean(content, 12000, "content");
        precautions = Clean(precautions, 4000, "precautions");
        if (keywords is not { Length: > 0 and <= 20 }) throw new ArgumentException("Use 1 to 20 keywords.");
        if (checks is not { Length: > 0 and <= 10 }) throw new ArgumentException("Use 1 to 10 completion checks.");
        string[] terms = keywords.Select(k => Clean(k, 80, "keyword").Replace(',', ' ').Replace('\r', ' ').Replace('\n', ' '))
            .Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
        string[] conditions = checks.Select(check => Clean(check, 240, "check").Replace('\r', ' ').Replace('\n', ' '))
            .Distinct(StringComparer.Ordinal).ToArray();
        if (!Directory.Exists(files.Root)) throw new DirectoryNotFoundException("Project is not open.");
        Directory.CreateDirectory(files.Resolve(Folder));
        for (int attempt = 0; attempt < 10; attempt++)
        {
            string id = Convert.ToHexString(SHA256.HashData(RandomNumberGenerator.GetBytes(32))).ToLowerInvariant();
            string relative = $"{Folder}/{fileTitle}_{id}.md";
            string path = files.Resolve(relative);
            string text = $"# {title}\n\nid: {id}\naliases: {string.Join(", ", terms)}\n\n## 내용\n{content}\n\n## 완료 확인\n{string.Join("\n", conditions.Select(check => "- " + check))}\n\n## 주의사항\n{precautions}\n";
            try
            {
                using var stream = new FileStream(path, FileMode.CreateNew, FileAccess.Write, FileShare.None);
                byte[] bytes = new UTF8Encoding(false).GetBytes(text);
                stream.Write(bytes);
                stream.Flush(true);
                return (id, relative);
            }
            catch (IOException) when (File.Exists(path)) { }
        }
        throw new IOException("Could not allocate a unique work-item ID.");
    }

    internal WikiSearchService.Result Search(string query, int limit, CancellationToken token)
    {
        var result = search.Search(query, limit, token);
        return result with { Matches = result.Matches
            .Where(h => h.Document.StartsWith("wiki/WorkItems/", StringComparison.Ordinal))
            .Select(h => h with { Document = "project-" + h.Document }).ToArray() };
    }

    internal WikiSearchService.Result List(int limit = 100)
    {
        string directory = files.Resolve(Folder);
        if (!Directory.Exists(directory))
            return new WikiSearchService.Result([], 0, 0, false, []);
        var clock = System.Diagnostics.Stopwatch.StartNew();
        var documents = new DirectoryInfo(directory).EnumerateFiles("*.md", SearchOption.TopDirectoryOnly)
            .Where(file => (file.Attributes & FileAttributes.ReparsePoint) == 0)
            .OrderByDescending(file => file.CreationTimeUtc).ToArray();
        var hits = documents.Take(limit).Select(file =>
        {
            string name = Path.GetFileNameWithoutExtension(file.Name);
            int separator = name.LastIndexOf('_');
            string title = separator > 0 && name.Length - separator - 1 == 64 ? name[..separator] : name;
            return new WikiSearchService.Hit("project-wiki/WorkItems/" + file.Name, title, 1, "", 0);
        }).ToArray();
        return new WikiSearchService.Result(hits, clock.Elapsed.TotalMilliseconds, documents.Length,
            documents.Length > limit, []);
    }

    internal string Read(string document)
    {
        return ReadSnapshot(document).Content;
    }

    internal (string Content, string Sha256) ReadSnapshot(string document)
    {
        string path = ResolveDocument(document);
        byte[] bytes = ProjectFileAccess.ReadBounded(path, 1_048_576);
        return (new UTF8Encoding(false, true).GetString(bytes).TrimStart('\uFEFF'),
            Convert.ToHexString(SHA256.HashData(bytes)));
    }

    internal (string UpdateId, string Sha256) AppendUpdate(string document, string expectedSha256,
        string changeType, string summary, string[] changedFiles)
    {
        if (expectedSha256.Length != 64 || expectedSha256.Any(c => !Uri.IsHexDigit(c)))
            throw new ArgumentException("A 64-character SHA256 is required.", nameof(expectedSha256));
        if (changeType is not ("기능변경" or "코드개선" or "버그수정"))
            throw new ArgumentException("Change type must be 기능변경, 코드개선 or 버그수정.", nameof(changeType));
        summary = summary.Trim().Replace('\r', ' ').Replace('\n', ' ');
        if (summary.Length is < 1 or > 2000) throw new ArgumentException("Summary must be 1-2000 characters.", nameof(summary));
        if (changedFiles is not { Length: > 0 and <= 20 }) throw new ArgumentException("Use 1-20 changed files.", nameof(changedFiles));
        string[] paths = changedFiles.Select(path => path.Replace('\\', '/')).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
        foreach (string path in paths)
        {
            if (!path.StartsWith("Asset/", StringComparison.OrdinalIgnoreCase))
                throw new ArgumentException("Changed files must be under Asset/.", nameof(changedFiles));
            files.Resolve(path);
        }
        string target = ResolveDocument(document);
        using var stream = new FileStream(target, FileMode.Open, FileAccess.ReadWrite, FileShare.None);
        if (stream.Length > 1_048_576) throw new IOException("Work item exceeds 1 MiB.");
        byte[] existing = new byte[checked((int)stream.Length)];
        stream.ReadExactly(existing);
        string currentHash = Convert.ToHexString(SHA256.HashData(existing));
        if (!currentHash.Equals(expectedSha256, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("Work item changed. Read it again before updating.");
        string updateId = Guid.NewGuid().ToString("N");
        string entry = $"\n## 변경 이력 · {DateTimeOffset.Now:yyyy-MM-dd HH:mm:ss zzz}\n" +
            $"update: {updateId}\n유형: {changeType}\n내용: {summary}\n변경 파일:\n" +
            string.Join("\n", paths.Select(path => "- " + path)) + "\n";
        byte[] appended = new UTF8Encoding(false).GetBytes(entry);
        if (existing.Length + appended.Length > 1_048_576) throw new IOException("Work item exceeds 1 MiB.");
        stream.Write(appended);
        stream.Flush(true);
        byte[] combined = new byte[existing.Length + appended.Length];
        existing.CopyTo(combined, 0);
        appended.CopyTo(combined, existing.Length);
        return (updateId, Convert.ToHexString(SHA256.HashData(combined)));
    }

    private string ResolveDocument(string document)
    {
        if (!document.StartsWith("project-wiki/WorkItems/", StringComparison.Ordinal))
            throw new ArgumentException("Invalid project work-item document.");
        string relative = document["project-wiki/".Length..];
        string name = relative["WorkItems/".Length..];
        if (!name.EndsWith(".md", StringComparison.Ordinal))
            throw new ArgumentException("Invalid project work-item ID.");
        string stem = name[..^3];
        int separator = stem.LastIndexOf('_');
        string id = separator < 0 ? stem : stem[(separator + 1)..];
        if (id.Length != 64 || id.Any(c => c is not (>= '0' and <= '9' or >= 'a' and <= 'f')) ||
            (separator >= 0 && (separator == 0 || stem[..separator].Length > 80)))
            throw new ArgumentException("Invalid project work-item ID.");
        string path = files.Resolve("Editor/" + relative);
        if (!File.Exists(path)) throw new FileNotFoundException("Work item not found.", path);
        return path;
    }
}
