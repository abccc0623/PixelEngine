using System.Diagnostics;
using System.IO;
using System.Text.Json;

namespace PixelTool;

// Observes the selected project only. This is guidance plus a narrow root-clutter guard,
// never a migration, a folder creator, or an authorization source for exceptions.
internal sealed class ProjectAssetLayout
{
    internal const int ResponseCharacterLimit = 16 * 1024;
    private const int MaximumEntries = 4000;
    private const int MaximumMilliseconds = 2000;
    private const int MaximumCandidates = 4;
    private const int CandidateCharacterBudget = 1024;
    private readonly ProjectFileAccess _files;
    private readonly int _entryLimit;
    private readonly int _milliseconds;

    private sealed record Category(string Name, string[] Extensions, string[] FolderNames);
    private static readonly Category[] Categories =
    [
        new("Script", [".lua", ".pxm"], ["Script", "Scripts"]),
        new("Scene", [".scene"], ["Scene", "Scenes"]),
        new("Texture", [".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tga", ".dds", ".tif", ".tiff", ".webp", ".svg"], ["Texture", "Textures", "Image", "Images"]),
        new("Table", [".csv", ".tsv", ".json", ".xml", ".yaml", ".yml", ".toml", ".db", ".sqlite", ".xlsx"], ["Table", "Tables", "Data"]),
        new("Audio", [".wav", ".ogg", ".mp3", ".flac", ".aac", ".m4a", ".opus", ".wma", ".aiff"], ["Audio", "Sound", "Sounds"]),
        new("Font", [".ttf", ".otf", ".woff", ".woff2", ".fnt"], ["Font", "Fonts"])
    ];

    private sealed class Observation
    {
        internal readonly List<string> Folders = [];
        internal int CandidateCharacters;
        internal int Files;
        internal int RootFiles;
        internal bool CandidatesTruncated;

        internal void AddFolder(string relative)
        {
            if (Folders.Contains(relative, StringComparer.OrdinalIgnoreCase)) return;
            int cost = JsonSerializer.Serialize(relative).Length;
            if (Folders.Count >= MaximumCandidates || CandidateCharacters + cost > CandidateCharacterBudget)
            {
                CandidatesTruncated = true;
                return;
            }
            Folders.Add(relative);
            CandidateCharacters += cost;
        }
    }

    private sealed class Snapshot
    {
        internal string AssetRoot = "Asset";
        internal bool AssetExists;
        internal bool Complete = true;
        internal bool Truncated;
        internal bool HasStructure;
        internal string? StopReason;
        internal int ExaminedEntries;
        internal int SkippedEntries;
        internal readonly Observation[] Categories = ProjectAssetLayout.Categories.Select(_ => new Observation()).ToArray();

        internal void Skip(string reason)
        {
            Complete = false;
            SkippedEntries++;
            StopReason ??= reason;
        }
    }

    // Limits can be reduced for callers/tests, never expanded beyond the shared ceiling.
    internal ProjectAssetLayout(string project, int entryLimit = MaximumEntries, int milliseconds = MaximumMilliseconds)
    {
        if (entryLimit is < 1 or > MaximumEntries) throw new ArgumentOutOfRangeException(nameof(entryLimit));
        if (milliseconds is < 0 or > MaximumMilliseconds) throw new ArgumentOutOfRangeException(nameof(milliseconds));
        _files = new ProjectFileAccess(project);
        _entryLimit = entryLimit;
        _milliseconds = milliseconds;
    }

    internal string GetInfo()
    {
        Snapshot snapshot = Scan();
        return JsonSerializer.Serialize(new
        {
            assetRoot = snapshot.AssetRoot,
            assetExists = snapshot.AssetExists,
            layoutState = snapshot.HasStructure ? "existing" : snapshot.Complete ? "blank" : "unknown",
            isBlank = snapshot.Complete ? (bool?)!snapshot.HasStructure : null,
            complete = snapshot.Complete,
            truncated = snapshot.Truncated,
            stopReason = snapshot.StopReason,
            examinedEntries = snapshot.ExaminedEntries,
            skippedEntries = snapshot.SkippedEntries,
            readOnly = true,
            categories = Categories.Select((category, index) => new
            {
                category = category.Name,
                extensions = category.Extensions,
                defaultFolder = snapshot.AssetRoot + "/" + category.Name,
                observedCandidateFolders = snapshot.Categories[index].Folders,
                candidatesTruncated = snapshot.Categories[index].CandidatesTruncated,
                observedFileCount = snapshot.Categories[index].Files,
                rootFileCount = snapshot.Categories[index].RootFiles
            }),
            guidance = "Use observed folders and preserve their spelling, casing and feature organization. Defaults are only fallbacks for categories without an established location; create only needed folders. main.lua may stay at Asset root. Never move existing assets automatically. This snapshot reads names and attributes only, not file contents or AI notes. Incomplete scans do not establish that a project is blank."
        });
    }

    // Returns a planning issue only. The host owns any visible, user-approved exception flow.
    // Each call observes current disk state: a previously authorized root creation is thereafter
    // an observed flat layout. Hosts needing one-shot exceptions should retain their own baseline.
    internal string? GetCreationIssue(string relative)
    {
        string path = _files.Resolve(relative);
        string normalized = Path.GetRelativePath(_files.Root, path).Replace('\\', '/');
        if (File.Exists(path)) return null;
        string[] parts = normalized.Split('/');
        if (parts.Length != 2 || !parts[0].Equals("Asset", StringComparison.OrdinalIgnoreCase)) return null;
        string name = parts[1];
        if (name.Equals("main.lua", StringComparison.OrdinalIgnoreCase)) return null;
        int categoryIndex = FindCategory(Path.GetExtension(name));
        if (categoryIndex < 0) return null;

        Snapshot snapshot = Scan();
        Observation observed = snapshot.Categories[categoryIndex];
        if (observed.RootFiles > 0) return null;
        string displayName = name.Length <= 255 ? name : name[..255] + "...";
        string[] suggestions = observed.Folders.Select(folder => folder + "/" + displayName).ToArray();
        string recommendation = suggestions.Length > 0
            ? "Observed candidate paths: " + string.Join(", ", suggestions) + "."
            : observed.CandidatesTruncated
                ? "Existing candidate folders were omitted by response limits; inspect the existing layout before choosing a path."
                : "Suggested path: " + snapshot.AssetRoot + "/" + Categories[categoryIndex].Name + "/" + displayName + ".";
        return "New " + Categories[categoryIndex].Name + " assets should not be placed directly in " + snapshot.AssetRoot +
            " without an established same-category root layout. " + recommendation +
            (snapshot.Complete ? "" : " The bounded scan is incomplete; do not assume this project is blank.") +
            " Preserve existing files. A root-path exception requires the host's explicit user approval flow.";
    }

    private static int FindCategory(string extension)
        => Array.FindIndex(Categories, category => category.Extensions.Contains(extension, StringComparer.OrdinalIgnoreCase));

    private Snapshot Scan()
    {
        var result = new Snapshot();
        var watch = Stopwatch.StartNew();
        bool Visit()
        {
            if (result.ExaminedEntries >= _entryLimit || watch.ElapsedMilliseconds >= _milliseconds)
            {
                result.Complete = false;
                result.Truncated = true;
                result.StopReason = result.ExaminedEntries >= _entryLimit ? "entry_limit" : "time_limit";
                return false;
            }
            result.ExaminedEntries++;
            return true;
        }

        // Resolve the real Asset spelling without presuming case-sensitive filesystem behavior.
        string? asset = null;
        string root = _files.Resolve(".");
        try
        {
            if (!Directory.Exists(root))
            {
                result.Skip("project_unavailable");
                return result;
            }
            foreach (string entry in Directory.EnumerateFileSystemEntries(root))
            {
                if (!Visit()) return result;
                if (!Path.GetFileName(entry).Equals("Asset", StringComparison.OrdinalIgnoreCase)) continue;
                result.AssetRoot = Path.GetFileName(entry);
                asset = _files.Resolve(result.AssetRoot);
                if (!Directory.Exists(asset))
                {
                    result.Skip("asset_not_directory");
                    return result;
                }
                result.AssetExists = true;
                break;
            }
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidOperationException)
        {
            result.Skip("asset_unavailable");
            return result;
        }
        if (asset == null)
        {
            result.Skip("asset_missing");
            return result;
        }

        var pending = new Queue<string>();
        pending.Enqueue(asset);
        while (pending.TryDequeue(out string? directory))
        {
            if (watch.ElapsedMilliseconds >= _milliseconds)
            {
                result.Complete = false;
                result.Truncated = true;
                result.StopReason = "time_limit";
                return result;
            }
            try
            {
                // Recheck at traversal time, not only when the directory was queued.
                directory = _files.Resolve(Path.GetRelativePath(_files.Root, directory));
                foreach (string entry in Directory.EnumerateFileSystemEntries(directory))
                {
                    if (!Visit()) return result;
                    string name = Path.GetFileName(entry);
                    if (ProjectFileAccess.Blocked(name))
                    {
                        result.Skip("protected_entries");
                        continue;
                    }
                    try
                    {
                        string relative = Path.GetRelativePath(_files.Root, entry).Replace('\\', '/');
                        string safePath = _files.Resolve(relative);
                        FileAttributes attributes = File.GetAttributes(safePath);
                        if ((attributes & FileAttributes.ReparsePoint) != 0)
                        {
                            result.Skip("unavailable_entries");
                            continue;
                        }
                        if ((attributes & FileAttributes.Directory) != 0)
                        {
                            result.HasStructure = true;
                            for (int i = 0; i < Categories.Length; i++)
                                if (Categories[i].FolderNames.Contains(name, StringComparer.OrdinalIgnoreCase))
                                    result.Categories[i].AddFolder(relative);
                            pending.Enqueue(safePath);
                            continue;
                        }
                        bool rootFile = directory.Equals(asset, StringComparison.OrdinalIgnoreCase);
                        if (rootFile && name.Equals("main.lua", StringComparison.OrdinalIgnoreCase)) continue;
                        result.HasStructure = true;
                        int index = FindCategory(Path.GetExtension(name));
                        if (index < 0) continue;
                        Observation observed = result.Categories[index];
                        observed.Files++;
                        if (rootFile) observed.RootFiles++;
                        observed.AddFolder(Path.GetRelativePath(_files.Root, directory).Replace('\\', '/'));
                    }
                    catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidOperationException)
                    {
                        result.Skip("unavailable_entries");
                    }
                }
            }
            catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidOperationException)
            {
                result.Skip("unavailable_directories");
            }
        }
        return result;
    }
}
