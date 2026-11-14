using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace PixelTool;

// Rules are project observations, not instructions for the assistant.
internal sealed class ProjectGameRuleWiki(string project)
{
    internal const string Folder = "Editor/GameRules";
    private readonly ProjectFileAccess files = new(project);
    private readonly WikiSearchService search = new(Path.Combine(project, "Editor"), ["GameRules"]);

    internal (string Document, string Sha256, bool Changed) Save(string category, string subject, string title,
        string rule, string[] keys, string basis, string status, string expectedSha256)
    {
        static string Clean(string value, int max, string field)
        {
            value = value.Trim();
            if (value.Length < 1 || value.Length > max || value.Any(c => char.IsControl(c) && c is not ('\n' or '\r' or '\t')))
                throw new ArgumentException($"Invalid {field}.");
            return value;
        }
        if (category is not ("global" or "object" or "input")) throw new ArgumentException("Invalid rule category.");
        if (status is not ("confirmed" or "inferred")) throw new ArgumentException("Invalid rule status.");
        subject = Clean(subject, 120, nameof(subject)).Replace('\r', ' ').Replace('\n', ' ');
        title = Clean(title, 160, nameof(title)).Replace('\r', ' ').Replace('\n', ' ');
        rule = Clean(rule, 4000, nameof(rule));
        basis = Clean(basis, 2000, nameof(basis));
        if (keys.Length > 10 || keys.Any(string.IsNullOrWhiteSpace)) throw new ArgumentException("Use 0-10 input keys.");
        string[] inputKeys = keys.Select(key => Clean(key, 32, "key").Replace(',', ' '))
            .Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
        if (expectedSha256.Length != 0 && (expectedSha256.Length != 64 || expectedSha256.Any(c => !Uri.IsHexDigit(c))))
            throw new ArgumentException("Expected SHA256 must be empty or 64 hexadecimal characters.");
        if (!Directory.Exists(files.Root)) throw new DirectoryNotFoundException("Project is not open.");

        string id = Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(
            category + "\n" + subject.ToUpperInvariant() + "\n" + title.ToUpperInvariant()))).ToLowerInvariant();
        string fileTitle = new string(title.Select(c => Path.GetInvalidFileNameChars().Contains(c) ? '_' : c).ToArray())
            .Trim(' ', '.', '_');
        if (fileTitle.Length > 80) fileTitle = fileTitle[..80].TrimEnd(' ', '.', '_');
        if (fileTitle.Length == 0) fileTitle = "규칙";
        string relative = $"GameRules/{category}/{fileTitle}_{id}.md";
        string document = "project-wiki/" + relative;
        string path = files.Resolve("Editor/" + relative);
        string content = $"# {title}\n\nid: {id}\ncategory: {category}\nsubject: {subject}\nstatus: {status}\n" +
            $"aliases: {subject}, {string.Join(", ", inputKeys)}\n\n## 규칙\n{rule}\n\n## 입력 키\n" +
            (inputKeys.Length == 0 ? "없음" : string.Join(", ", inputKeys)) + $"\n\n## 근거\n{basis}\n";
        byte[] replacement = new UTF8Encoding(false).GetBytes(content);
        string nextHash = Convert.ToHexString(SHA256.HashData(replacement));
        Directory.CreateDirectory(files.Resolve($"{Folder}/{category}"));
        if (expectedSha256.Length != 0 && !File.Exists(path))
            throw new InvalidOperationException("Game rule was removed. Search again before saving.");

        try
        {
            using var created = new FileStream(path, FileMode.CreateNew, FileAccess.Write, FileShare.None);
            created.Write(replacement);
            created.Flush(true);
            return (document, nextHash, true);
        }
        catch (IOException) when (File.Exists(path)) { }

        using var stream = new FileStream(path, FileMode.Open, FileAccess.ReadWrite, FileShare.None);
        if (stream.Length > 1_048_576) throw new IOException("Game rule exceeds 1 MiB.");
        byte[] existing = new byte[checked((int)stream.Length)];
        stream.ReadExactly(existing);
        string currentHash = Convert.ToHexString(SHA256.HashData(existing));
        if (currentHash.Equals(nextHash, StringComparison.OrdinalIgnoreCase)) return (document, currentHash, false);
        if (!currentHash.Equals(expectedSha256, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("Game rule changed. Read it again before updating.");
        stream.Position = 0;
        stream.SetLength(0);
        stream.Write(replacement);
        stream.Flush(true);
        return (document, nextHash, true);
    }

    internal WikiSearchService.Result Search(string query, int limit, CancellationToken token)
    {
        var result = search.Search(query, limit, token);
        return result with { Matches = result.Matches.Select(hit => hit with
            { Document = "project-" + hit.Document }).ToArray() };
    }

    internal WikiSearchService.Result List(int limit = 100)
    {
        string directory = files.Resolve(Folder);
        if (!Directory.Exists(directory)) return new([], 0, 0, false, []);
        var clock = System.Diagnostics.Stopwatch.StartNew();
        var documents = new[] { "global", "object", "input" }
            .Select(category => files.Resolve($"{Folder}/{category}"))
            .Where(Directory.Exists)
            .SelectMany(folder => new DirectoryInfo(folder).EnumerateFiles("*.md", SearchOption.TopDirectoryOnly))
            .Where(file => (file.Attributes & FileAttributes.ReparsePoint) == 0)
            .OrderByDescending(file => file.LastWriteTimeUtc).ToArray();
        var hits = documents.Take(limit).Select(file =>
        {
            string name = Path.GetFileNameWithoutExtension(file.Name);
            int separator = name.LastIndexOf('_');
            string title = separator > 0 && name.Length - separator - 1 == 64 ? name[..separator] : name;
            return new WikiSearchService.Hit(
                "project-wiki/GameRules/" + Path.GetRelativePath(directory, file.FullName).Replace('\\', '/'),
                title, 1, "", 0);
        }).ToArray();
        return new(hits, clock.Elapsed.TotalMilliseconds, documents.Length, documents.Length > limit, []);
    }

    internal (string Content, string Sha256) ReadSnapshot(string document)
    {
        if (!document.StartsWith("project-wiki/GameRules/", StringComparison.Ordinal))
            throw new ArgumentException("Invalid project game-rule document.");
        string relative = document["project-wiki/".Length..];
        string[] parts = relative.Split('/');
        if (parts.Length != 3 || parts[0] != "GameRules" || parts[1] is not ("global" or "object" or "input") ||
            !parts[2].EndsWith(".md", StringComparison.Ordinal) || parts[2].Contains('\\'))
            throw new ArgumentException("Invalid project game-rule document.");
        string stem = parts[2][..^3];
        int separator = stem.LastIndexOf('_');
        if (separator < 1 || stem.Length - separator - 1 != 64 ||
            stem[(separator + 1)..].Any(c => c is not (>= '0' and <= '9' or >= 'a' and <= 'f')))
            throw new ArgumentException("Invalid project game-rule document.");
        string path = files.Resolve("Editor/" + relative);
        byte[] bytes = ProjectFileAccess.ReadBounded(path, 1_048_576);
        return (new UTF8Encoding(false, true).GetString(bytes).TrimStart('\uFEFF'),
            Convert.ToHexString(SHA256.HashData(bytes)));
    }
}
