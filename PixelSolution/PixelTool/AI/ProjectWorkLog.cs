using System.Globalization;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;

namespace PixelTool;

// A completion receipt, not an instruction source. Callers supply observed facts only.
internal static class ProjectWorkLog
{
    internal const string RelativePath = "Editor/AIWorkLog.md";
    private const int MaxBytes = 256 * 1024;
    private const string Header = "# AI work log v1\n\n";
    private static readonly UTF8Encoding Utf8 = new(false, true);

    internal sealed record Record(string WorkId, string Summary, string Reason,
        IReadOnlyList<string> ChangedPaths, string ValidationStatus, DateTimeOffset CompletedAt,
        TimeSpan? Duration = null, long? Tokens = null);
    internal sealed record Result(bool Saved, string Path, string Message);

    internal static Result Append(string projectRoot, Record record)
    {
        string? temporary = null;
        try
        {
            var access = new ProjectFileAccess(projectRoot);
            if (!Directory.Exists(access.Root)) throw new InvalidOperationException("Project root does not exist.");
            string path = access.Resolve(RelativePath);
            string entry = Format(access, record);
            byte[] addition = Utf8.GetBytes(entry);
            if (addition.Length > 4096) throw new InvalidOperationException("Work note exceeds 4 KiB.");
            using var gate = new Mutex(false, "PixelTool.WorkLog." + Hash(access.Root.ToUpperInvariant()));
            bool held;
            try { held = gate.WaitOne(TimeSpan.FromSeconds(5)); }
            catch (AbandonedMutexException) { held = true; }
            if (!held) return new(false, RelativePath, "Work log is busy; no note saved.");
            try
            {
                access.Resolve(RelativePath);
                bool exists = File.Exists(path);
                byte[] previous = exists ? ProjectFileAccess.ReadBounded(path, MaxBytes) : Utf8.GetBytes(Header);
                string text = Utf8.GetString(previous);
                ValidateExisting(access, text);
                if (text.Contains("<!-- work:" + Hash(record.WorkId) + " -->\n", StringComparison.Ordinal))
                    return new(true, RelativePath, "Work note already saved.");
                if (previous.Length + addition.Length > MaxBytes)
                    return new(false, RelativePath, "Work log reached 256 KiB; previous notes preserved, new note not saved.");
                Directory.CreateDirectory(access.Resolve("Editor"));
                temporary = access.Resolve("Editor/AIWorkLog-" + Guid.NewGuid().ToString("N") + ".tmp");
                using (var file = new FileStream(temporary, FileMode.CreateNew, FileAccess.Write, FileShare.None))
                {
                    file.Write(previous);
                    file.Write(addition);
                    file.Flush(true);
                }
                access.Resolve(RelativePath);
                access.Resolve(Path.GetRelativePath(access.Root, temporary));
                // Avoid replacing ordinary user edits made during preparation.
                if (File.Exists(path) != exists || (exists && !ProjectFileAccess.ReadBounded(path, MaxBytes).AsSpan().SequenceEqual(previous)))
                    return new(false, RelativePath, "Work log changed during save; no note saved.");
                if (exists) File.Replace(temporary, path, null);
                else File.Move(temporary, path);
                temporary = null;
                return new(true, RelativePath, "Work note saved.");
            }
            finally { gate.ReleaseMutex(); }
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or ArgumentException
            or InvalidOperationException or NotSupportedException)
        {
            return new(false, RelativePath, "Work note not saved: " + ex.Message);
        }
        finally
        {
            if (temporary != null)
            {
                try
                {
                    var access = new ProjectFileAccess(projectRoot);
                    File.Delete(access.Resolve(Path.GetRelativePath(access.Root, temporary)));
                }
                catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or ArgumentException or InvalidOperationException) { }
            }
        }
    }

    private static string Format(ProjectFileAccess access, Record record)
    {
        ArgumentNullException.ThrowIfNull(record);
        string id = Plain(record.WorkId, 200);
        string summary = Plain(record.Summary, 500);
        string reason = Plain(record.Reason, 500);
        string validation = Plain(record.ValidationStatus, 500);
        if (record.CompletedAt == default || record.Duration < TimeSpan.Zero || record.Tokens < 0)
            throw new ArgumentException("Invalid completion time, duration or token count.");
        if (record.ChangedPaths == null || record.ChangedPaths.Count > 20)
            throw new ArgumentException("At most 20 verified changed paths are allowed.");
        var paths = new List<string>();
        foreach (string relative in record.ChangedPaths)
        {
            string value = Plain(relative, 240).Replace('\\', '/');
            if (value.Split('/').Any(part => part is "" or "." or ".." || part.EndsWith('.') || part.EndsWith(' ')))
                throw new ArgumentException("Changed paths must be canonical project-relative paths.");
            access.Resolve(value);
            if (!paths.Contains(value, StringComparer.OrdinalIgnoreCase)) paths.Add(value);
        }
        var output = new StringBuilder();
        output.Append("<!-- work:").Append(Hash(id)).Append(" -->\n");
        output.Append("## ").Append(record.CompletedAt.ToUniversalTime().ToString("O", CultureInfo.InvariantCulture)).Append('\n');
        output.Append("- What: ").Append(summary).Append('\n');
        output.Append("- Why: ").Append(reason).Append('\n');
        output.Append("- Validation: ").Append(validation).Append('\n');
        foreach (string path in paths) output.Append("- Changed: ").Append(path).Append('\n');
        if (record.Duration.HasValue) output.Append("- Validation elapsed seconds: ").Append(record.Duration.Value.TotalSeconds.ToString("0.###", CultureInfo.InvariantCulture)).Append('\n');
        if (record.Tokens.HasValue) output.Append("- Tokens: ").Append(record.Tokens.Value.ToString(CultureInfo.InvariantCulture)).Append('\n');
        return output.Append("<!-- end work -->\n\n").ToString();
    }

    private static string Plain(string value, int maximum)
    {
        if (string.IsNullOrWhiteSpace(value) || value.Length > maximum || value != value.Trim()
            || value.Any(c => char.IsControl(c) || char.GetUnicodeCategory(c) == UnicodeCategory.Format)
            || value.IndexOfAny(['<', '>', '[', ']', '`']) >= 0 || value.Contains("://", StringComparison.Ordinal))
            throw new ArgumentException("Work notes must be bounded single-line plain text without links or markup.");
        return value;
    }

    private static void ValidateExisting(ProjectFileAccess access, string text)
    {
        if (!text.StartsWith(Header, StringComparison.Ordinal)) throw new InvalidOperationException("Unrecognized work log format; existing file preserved.");
        string remaining = text[Header.Length..];
        var ids = new HashSet<string>(StringComparer.Ordinal);
        while (remaining.Length > 0)
        {
            int end = remaining.IndexOf("<!-- end work -->\n\n", StringComparison.Ordinal);
            if (end < 0) throw new InvalidOperationException("Incomplete work log; existing file preserved.");
            string block = remaining[..(end + "<!-- end work -->\n\n".Length)];
            string[] lines = block.Split('\n');
            if (lines.Length < 8 || Utf8.GetByteCount(block) > 4096 || !Regex.IsMatch(lines[0], @"\A<!-- work:[A-F0-9]{64} -->\z")
                || !ids.Add(lines[0]) || !lines[1].StartsWith("## ", StringComparison.Ordinal)
                || !DateTimeOffset.TryParseExact(lines[1][3..], "O", CultureInfo.InvariantCulture, DateTimeStyles.None, out _)
                || !lines[2].StartsWith("- What: ", StringComparison.Ordinal)
                || !lines[3].StartsWith("- Why: ", StringComparison.Ordinal)
                || !lines[4].StartsWith("- Validation: ", StringComparison.Ordinal))
                throw new InvalidOperationException("Malformed work log; existing file preserved.");
            Plain(lines[2][8..], 500);
            Plain(lines[3][7..], 500);
            Plain(lines[4][14..], 500);
            int changed = 0;
            foreach (string line in lines.Skip(5).Take(lines.Length - 8))
            {
                if (line.StartsWith("- Changed: ", StringComparison.Ordinal))
                {
                    string relative = Plain(line[11..], 240).Replace('\\', '/');
                    if (relative.Split('/').Any(part => part is "" or "." or ".." || part.EndsWith('.') || part.EndsWith(' ')))
                        throw new InvalidOperationException("Malformed work log path; existing file preserved.");
                    access.Resolve(relative);
                    changed++;
                }
                else if (line.StartsWith("- Validation elapsed seconds: ", StringComparison.Ordinal)
                    && double.TryParse(line["- Validation elapsed seconds: ".Length..], NumberStyles.AllowDecimalPoint, CultureInfo.InvariantCulture, out double elapsed) && double.IsFinite(elapsed) && elapsed >= 0) { }
                else if (line.StartsWith("- Duration seconds: ", StringComparison.Ordinal)
                    && double.TryParse(line[20..], NumberStyles.AllowDecimalPoint, CultureInfo.InvariantCulture, out double duration) && double.IsFinite(duration) && duration >= 0) { }
                else if (line.StartsWith("- Tokens: ", StringComparison.Ordinal)
                    && long.TryParse(line[10..], NumberStyles.None, CultureInfo.InvariantCulture, out _)) { }
                else throw new InvalidOperationException("Malformed work log field; existing file preserved.");
            }
            if (changed > 20) throw new InvalidOperationException("Malformed work log paths; existing file preserved.");
            remaining = remaining[block.Length..];
        }
    }

    private static string Hash(string text) => Convert.ToHexString(SHA256.HashData(Utf8.GetBytes(text)));
}
