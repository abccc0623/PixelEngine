using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;

namespace PixelTool
{
    internal sealed class AssetBrowserState
    {
        public AssetBrowserState() { }
        public Dictionary<string, bool> Expanded { get; set; } = new(StringComparer.OrdinalIgnoreCase);
        public string Selected { get; set; } = ".";

        public static AssetBrowserState Load(string path)
        {
            if (!File.Exists(path)) return new AssetBrowserState();
            var state = JsonSerializer.Deserialize<AssetBrowserState>(File.ReadAllText(path)) ?? new AssetBrowserState();
            state.Expanded = new Dictionary<string, bool>(state.Expanded ?? new(), StringComparer.OrdinalIgnoreCase);
            state.Selected ??= ".";
            return state;
        }

        public void Save(string path)
        {
            Directory.CreateDirectory(Path.GetDirectoryName(path)!);
            string temporary = path + ".tmp";
            File.WriteAllText(temporary, JsonSerializer.Serialize(this));
            File.Move(temporary, path, true);
        }
    }

    internal static class AssetFileOperations
    {
        public static bool IsInside(string path, string root)
        {
            string full = Path.TrimEndingDirectorySeparator(Path.GetFullPath(path));
            string parent = Path.TrimEndingDirectorySeparator(Path.GetFullPath(root));
            return full.Equals(parent, StringComparison.OrdinalIgnoreCase) ||
                full.StartsWith(parent + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase);
        }

        public static void RequireAssetPath(string path, string root, bool allowRoot = false)
        {
            path = Path.TrimEndingDirectorySeparator(Path.GetFullPath(path));
            root = Path.TrimEndingDirectorySeparator(Path.GetFullPath(root));
            if (!IsInside(path, root) || (!allowRoot && path.Equals(root, StringComparison.OrdinalIgnoreCase)))
                throw new IOException("The operation must stay inside Asset; the root cannot be moved or deleted.");
            // Do not follow a junction/symlink into data outside the project.
            for (string? current = path; current != null && IsInside(current, root); current = Path.GetDirectoryName(current))
            {
                if ((File.Exists(current) || Directory.Exists(current)) &&
                    (File.GetAttributes(current) & FileAttributes.ReparsePoint) != 0)
                    throw new IOException("Linked folders/files cannot be changed by the Asset browser.");
                if (current.Equals(root, StringComparison.OrdinalIgnoreCase)) break;
            }
        }

        public static string NamedPath(string directory, string name)
        {
            if (string.IsNullOrWhiteSpace(name) || name == "." || name == ".." ||
                name.IndexOfAny(Path.GetInvalidFileNameChars()) >= 0 || name.EndsWith('.') || name.EndsWith(' '))
                throw new IOException("Enter a file or folder name, not a path.");
            return Path.Combine(directory, name);
        }

        public static string UniquePath(string directory, string name, string extension)
        {
            string path = NamedPath(directory, name + extension);
            for (int i = 1; File.Exists(path) || Directory.Exists(path); i++)
                path = NamedPath(directory, name + "_" + i + extension);
            return path;
        }

        public static List<(string Source, string Destination)> Transfer(IEnumerable<string> sources, string directory, bool move, string assetRoot, Action<string,string>? moved = null)
        {
            RequireAssetPath(directory, assetRoot, true);
            if (!Directory.Exists(directory)) throw new DirectoryNotFoundException(directory);
            var paths = sources.Select(Path.GetFullPath).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
            // Selecting a parent and its child must not transfer the child twice.
            paths = paths.Where(path => !paths.Any(parent => !parent.Equals(path, StringComparison.OrdinalIgnoreCase) &&
                Directory.Exists(parent) && IsInside(path, parent))).ToArray();
            var completed = new List<(string Source, string Destination)>();
            foreach (string source in paths)
            {
                if (!File.Exists(source) && !Directory.Exists(source)) continue;
                bool internalSource = IsInside(source, assetRoot);
                bool shouldMove = move && internalSource;
                if (internalSource) RequireAssetPath(source, assetRoot);
                if ((File.GetAttributes(source) & FileAttributes.ReparsePoint) != 0)
                    throw new IOException("Linked files/folders cannot be imported.");
                if (Directory.Exists(source) && IsInside(directory, source))
                    throw new IOException("A folder cannot be copied or moved into itself.");
                string destination = Path.Combine(directory, Path.GetFileName(source));
                if (source.Equals(destination, StringComparison.OrdinalIgnoreCase) && shouldMove) continue;
                if (File.Exists(destination) || Directory.Exists(destination))
                {
                    bool folder = Directory.Exists(source);
                    destination = UniquePath(directory, folder ? Path.GetFileName(source) : Path.GetFileNameWithoutExtension(source), folder ? "" : Path.GetExtension(source));
                }
                if (Directory.Exists(source))
                {
                    // Validate the complete tree before copying, so links cannot cause recursive loops.
                    CheckDirectory(source);
                    if (shouldMove) Directory.Move(source, destination);
                    else CopyDirectory(source, destination);
                }
                else if (shouldMove) File.Move(source, destination);
                else File.Copy(source, destination);
                completed.Add((source, destination));
                if (shouldMove) moved?.Invoke(source,destination);
            }
            return completed;
        }

        private static void CheckDirectory(string source)
        {
            foreach (string entry in Directory.EnumerateFileSystemEntries(source))
            {
                var attributes = File.GetAttributes(entry);
                if ((attributes & FileAttributes.ReparsePoint) != 0) throw new IOException("Linked files/folders cannot be imported.");
                if ((attributes & FileAttributes.Directory) != 0) CheckDirectory(entry);
            }
        }

        private static void CopyDirectory(string source, string destination)
        {
            Directory.CreateDirectory(destination);
            foreach (string file in Directory.EnumerateFiles(source)) File.Copy(file, Path.Combine(destination, Path.GetFileName(file)));
            foreach (string child in Directory.EnumerateDirectories(source)) CopyDirectory(child, Path.Combine(destination, Path.GetFileName(child)));
        }
    }
}
