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

    internal (string Id, string Path) Create(string title, string content, string precautions, string[] keywords)
    {
        static string Clean(string value, int max, string field)
        {
            value = value.Trim();
            if (value.Length is 0 or > 12000 || value.Length > max || value.Any(c => char.IsControl(c) && c is not ('\n' or '\r' or '\t')))
                throw new ArgumentException($"Invalid {field}.");
            return value;
        }
        title = Clean(title, 200, "title").Replace('\r', ' ').Replace('\n', ' ');
        content = Clean(content, 12000, "content");
        precautions = Clean(precautions, 4000, "precautions");
        if (keywords is not { Length: > 0 and <= 20 }) throw new ArgumentException("Use 1 to 20 keywords.");
        string[] terms = keywords.Select(k => Clean(k, 80, "keyword").Replace(',', ' ').Replace('\r', ' ').Replace('\n', ' '))
            .Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
        if (!Directory.Exists(files.Root)) throw new DirectoryNotFoundException("Project is not open.");
        Directory.CreateDirectory(files.Resolve(Folder));
        for (int attempt = 0; attempt < 10; attempt++)
        {
            string id = Convert.ToHexString(SHA256.HashData(RandomNumberGenerator.GetBytes(32))).ToLowerInvariant();
            string relative = $"{Folder}/{id}.md";
            string path = files.Resolve(relative);
            string text = $"# {title}\n\nid: {id}\naliases: {string.Join(", ", terms)}\n\n## 내용\n{content}\n\n## 주의사항\n{precautions}\n";
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

    internal string Read(string document)
    {
        if (!document.StartsWith("project-wiki/WorkItems/", StringComparison.Ordinal))
            throw new ArgumentException("Invalid project work-item document.");
        string relative = document["project-wiki/".Length..];
        string name = relative["WorkItems/".Length..];
        if (name.Length != 67 || !name.EndsWith(".md", StringComparison.Ordinal) ||
            name[..64].Any(c => c is not (>= '0' and <= '9' or >= 'a' and <= 'f')))
            throw new ArgumentException("Invalid project work-item ID.");
        string path = files.Resolve("Editor/" + relative);
        if (!File.Exists(path)) throw new FileNotFoundException("Work item not found.", path);
        return search.Read("wiki/" + relative);
    }
}
