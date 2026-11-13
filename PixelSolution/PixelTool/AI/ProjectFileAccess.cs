using System.IO;

namespace PixelTool;

// One path policy for chat tools, project navigation and the local MCP adapter.
internal sealed class ProjectFileAccess(string project)
{
    internal const string AssetMutationDenied = "Asset폴더외에 파일및 폴더수정은 불가능합니다.";
    // Local processing limit is separate from the much smaller model response budget.
    internal const int MaxEditableFileBytes = 4 * 1024 * 1024;
    private static readonly HashSet<string> CodeExtensions = new(StringComparer.OrdinalIgnoreCase)
        { ".lua", ".pxm", ".scene", ".cs", ".csx", ".c", ".h", ".cpp", ".hpp", ".cc", ".cxx", ".hxx",
          ".js", ".jsx", ".ts", ".tsx", ".mjs", ".cjs", ".py", ".pyw", ".rb", ".go", ".rs",
          ".java", ".kt", ".swift", ".sh", ".ps1", ".bat", ".cmd", ".hlsl", ".glsl", ".shader" };
    internal static bool IsCode(string path) => CodeExtensions.Contains(Path.GetExtension(path));
    internal string Root { get; } = Path.TrimEndingDirectorySeparator(Path.GetFullPath(project));
    internal static readonly HashSet<string> TextExtensions = new(StringComparer.OrdinalIgnoreCase)
        { ".lua", ".pxm", ".scene", ".json", ".md", ".txt", ".csv", ".xml", ".yaml", ".yml", ".svg" };

    internal string Resolve(string relative)
    {
        if (Path.IsPathRooted(relative) || relative.Contains(':')) throw new InvalidOperationException("프로젝트 상대 경로만 사용할 수 있습니다.");
        string path = Path.GetFullPath(Path.Combine(Root, relative));
        if (!path.Equals(Root, StringComparison.OrdinalIgnoreCase) && !path.StartsWith(Root + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("프로젝트 밖의 경로는 사용할 수 없습니다.");
        // Check every ancestor, including the project root; never follow a junction or symlink.
        for (string? current = path; current != null; current = Path.GetDirectoryName(current))
        {
            bool isRoot = current.Equals(Root, StringComparison.OrdinalIgnoreCase);
            if (!isRoot && Blocked(Path.GetFileName(current))) throw new InvalidOperationException("인증·비밀 파일이나 내부 관리 경로는 접근할 수 없습니다.");
            if ((File.Exists(current) || Directory.Exists(current)) && (File.GetAttributes(current) & FileAttributes.ReparsePoint) != 0)
                throw new InvalidOperationException("심볼릭 링크·정션 경로는 접근할 수 없습니다.");
            if (isRoot) break;
        }
        return path;
    }

    internal static bool Blocked(string name) => name.StartsWith('.') || name.Contains("token", StringComparison.OrdinalIgnoreCase)
        || name.Contains("secret", StringComparison.OrdinalIgnoreCase) || name.Contains("credential", StringComparison.OrdinalIgnoreCase)
        || name.Equals("auth.json", StringComparison.OrdinalIgnoreCase) || name.EndsWith(".pem", StringComparison.OrdinalIgnoreCase)
        || name.EndsWith(".key", StringComparison.OrdinalIgnoreCase);

    internal static byte[] ReadBounded(string path, int limit = 256 * 1024)
    {
        using var file = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
        if (file.Length > limit) throw new InvalidOperationException("파일이 허용 크기를 초과했습니다.");
        byte[] bytes = new byte[checked((int)file.Length)];
        file.ReadExactly(bytes);
        return bytes;
    }
}

// One response may expose only part of each code file, including repeated ranges in a batch.
// Separate requests may follow continuations; this is a token budget, not a confidentiality boundary.
internal sealed class ProjectCodeReadBudget
{
    private readonly Dictionary<string, (string Hash, int Remaining)> _files = new(StringComparer.OrdinalIgnoreCase);
    internal int Remaining(string path, string text, string hash)
    {
        if (!ProjectFileAccess.IsCode(path)) return int.MaxValue;
        if (!_files.TryGetValue(path, out var state))
        {
            int characters = text.Count(c => c is not ('\r' or '\n'));
            state = (hash, characters / 2);
            _files.Add(path, state);
        }
        if (state.Hash != hash) throw new InvalidOperationException("File changed during the read batch. Read the current range again.");
        return state.Remaining;
    }
    internal void Consume(string path, int characters)
    {
        if (_files.TryGetValue(path, out var state)) _files[path] = (state.Hash, state.Remaining - characters);
    }
}

internal static class ProjectTextPatch
{
    // Line reads use LF, while files may contain CRLF. Match either form without changing unrelated text.
    internal static bool TryReplace(string source, string oldText, string newText, out string result, out int occurrences)
    {
        string normalized = Normalize(oldText);
        string content = Normalize(source);
        int start = content.IndexOf(normalized, StringComparison.Ordinal);
        occurrences = start < 0 ? 0 : content.IndexOf(normalized, start + 1, StringComparison.Ordinal) < 0 ? 1 : 2;
        if (occurrences != 1)
        {
            result = source;
            return false;
        }

        int sourceStart = SourceOffset(source, start);
        int sourceEnd = SourceOffset(source, start + normalized.Length);
        bool crlf = source[sourceStart..sourceEnd].Contains("\r\n", StringComparison.Ordinal) || source.Contains("\r\n", StringComparison.Ordinal);
        string replacement = Normalize(newText);
        if (crlf) replacement = replacement.Replace("\n", "\r\n", StringComparison.Ordinal);
        result = source[..sourceStart] + replacement + source[sourceEnd..];
        return true;
    }

    private static string Normalize(string text) => text.Replace("\r\n", "\n", StringComparison.Ordinal).Replace('\r', '\n');

    private static int SourceOffset(string source, int normalizedOffset)
    {
        int offset = 0;
        for (int i = 0; i < normalizedOffset; i++)
        {
            if (source[offset] == '\r' && offset + 1 < source.Length && source[offset + 1] == '\n') offset++;
            offset++;
        }
        return offset;
    }
}
