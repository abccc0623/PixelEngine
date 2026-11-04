using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace PixelTool;

// Deliberately no arbitrary shell, deletion or network tool in the direct client.
internal sealed class ChatGptProjectTools(string project)
{
    private readonly string _root = Path.TrimEndingDirectorySeparator(Path.GetFullPath(project));
    private static readonly HashSet<string> TextExtensions = new(StringComparer.OrdinalIgnoreCase)
        { ".lua", ".pxm", ".scene", ".json", ".md", ".txt", ".csv", ".xml", ".yaml", ".yml", ".svg" };
    internal static readonly HashSet<string> AssetExtensions = new(StringComparer.OrdinalIgnoreCase)
        { ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds", ".wav", ".ogg", ".mp3", ".ttf", ".otf" };
    internal sealed record ImportedAsset(string Name, byte[] Bytes);

    internal static object[] Definitions => new object[]
    {
        Tool("ask_choice", "Ask the user one concrete question with 2 to 5 clickable options. Wait for their selection or custom text before continuing. Use for meaningful choices, not approvals or decisions already clear from project code.",
            new Dictionary<string, object> { ["question"] = new { type = "string", minLength = 1, maxLength = 500 },
                ["options"] = new { type = "array", minItems = 2, maxItems = 5,
                    items = new { type = "string", minLength = 1, maxLength = 150 } } }),
        Tool("reimport_project", "Request the editor's existing Reload path on its render thread, resetting runtime and reimporting Asset/main.lua. Refuses unsaved editor buffers or a paused debugger. Returns only after the reload frame completes, with a log cursor and captured logs. Inspect logs; completion is not proof of no runtime errors.",
            new Dictionary<string, object>()),
        Tool("read_editor_logs", "Read captured editor/engine logs newer than afterCursor (0 for recent history). Follow cursor/hasMore for more pages. Check truncated/cleared flags before claiming no errors.",
            new Dictionary<string, object> { ["afterCursor"] = new { type = "integer", minimum = 0 }, ["limit"] = new { type = "integer", minimum = 1, maximum = 100 } }),
        Tool("list_project_files", "List one project directory. Use a relative path, or . for the root.",
            new Dictionary<string, object> { ["path"] = new { type = "string" } }),
        Tool("read_project_file", "Read a project text file and its SHA256 for safe replacement. Paths are relative to the project.",
            new Dictionary<string, object> { ["path"] = new { type = "string" } }),
        Tool("write_project_file", "Create or replace one project text file under the current editor approval setting. Read first and supply its expected SHA256. For a new file use an empty expectedHash. Never delete.",
            new Dictionary<string, object> { ["path"] = new { type = "string" }, ["content"] = new { type = "string" }, ["expectedHash"] = new { type = "string" } }),
        Tool("create_asset_folder", "Create an Asset subdirectory. path must begin with Asset/.",
            new Dictionary<string, object> { ["path"] = new { type = "string" } }),
        Tool("inspect_asset", "Get a project image/audio/font file's size, extension and SHA256. Does not interpret image contents or play audio.",
            new Dictionary<string, object> { ["path"] = new { type = "string" } }),
        Tool("copy_asset", "Copy an existing project image/audio/font into Asset/. Never overwrite. source is project-relative; path is the new Asset destination.",
            new Dictionary<string, object> { ["path"] = new { type = "string" }, ["source"] = new { type = "string" } }),
        Tool("import_asset", "Ask the user to select an external image/audio/font with the editor file picker, then copy into Asset/. Never supply arbitrary outside paths. Destination extension must match the selected file. Never overwrite.",
            new Dictionary<string, object> { ["path"] = new { type = "string" } }),
        Tool("create_png_asset", "Create a simple pixel-art or placeholder PNG inside Asset/, with RGBA background and filled rectangles. Colors are #RRGGBBAA; transparent is #00000000. Rectangles are composited in order by replacement, not alpha blending. This is procedural drawing, not an image-generation model. Never overwrite. Maximum dimensions 1024x1024.",
            new Dictionary<string, object>
            {
                ["path"] = new { type = "string" }, ["width"] = new { type = "integer", minimum = 1, maximum = 1024 },
                ["height"] = new { type = "integer", minimum = 1, maximum = 1024 }, ["background"] = new { type = "string" },
                ["rectangles"] = new { type = "array", maxItems = 4096, items = new { type = "object", additionalProperties = false,
                    properties = new { x = new { type = "integer" }, y = new { type = "integer" }, width = new { type = "integer" }, height = new { type = "integer" }, color = new { type = "string" } },
                    required = new[] { "x", "y", "width", "height", "color" } } }
            })
    };

    private static object Tool(string name, string description, Dictionary<string, object> properties) => new
    {
        type = "function", name, description, strict = true,
        parameters = new { type = "object", properties, required = properties.Keys.ToArray(), additionalProperties = false }
    };

    internal async Task<string> InvokeAsync(string name, JsonElement arguments, bool allowEdits, bool confirmChanges,
        Func<string, string, string, Task<bool>> approve, CancellationToken token, Func<Task<ImportedAsset?>>? pickAsset = null,
        Action<string, string, string>? savedCode = null)
    {
        token.ThrowIfCancellationRequested();
        string relative = arguments.GetProperty("path").GetString() ?? "";
        string path = Resolve(relative);
        if (name == "list_project_files")
        {
            var entries = Directory.EnumerateFileSystemEntries(path)
                .Where(p => !Blocked(Path.GetFileName(p)) && (File.GetAttributes(p) & FileAttributes.ReparsePoint) == 0)
                .Take(250).Select(p => new { name = Path.GetFileName(p), directory = Directory.Exists(p) }).ToArray();
            return JsonSerializer.Serialize(new { entries, limit = 250 });
        }
        if (name == "inspect_asset")
        {
            CheckAssetExtension(path);
            byte[] bytes = ReadBounded(path, 16 * 1024 * 1024);
            return JsonSerializer.Serialize(new { path = relative, extension = Path.GetExtension(path), size = bytes.Length, sha256 = Hash(bytes) });
        }
        if (name is "create_asset_folder" or "copy_asset" or "import_asset" or "create_png_asset")
        {
            if (!allowEdits) return "파일 수정 허용이 꺼져 있습니다. 에셋을 생성하지 않았습니다.";
            ResolveAsset(relative);
            if (name == "create_asset_folder")
            {
                if (Directory.Exists(path)) return "폴더가 이미 존재합니다. 변경하지 않았습니다.";
                if (confirmChanges && !await approve(relative, "", "에셋 폴더 생성").ConfigureAwait(false)) return "폴더 생성을 승인하지 않았습니다.";
                token.ThrowIfCancellationRequested();
                Directory.CreateDirectory(ResolveAsset(relative));
                return JsonSerializer.Serialize(new { created = relative });
            }
            if (File.Exists(path) || Directory.Exists(path)) return "대상이 이미 존재합니다. 에셋은 자동으로 덮어쓰지 않습니다.";
            CheckAssetExtension(path);
            byte[] asset;
            string description;
            if (name == "create_png_asset")
            {
                if (!Path.GetExtension(path).Equals(".png", StringComparison.OrdinalIgnoreCase)) throw new InvalidOperationException("PNG 경로만 사용할 수 있습니다.");
                asset = CreatePng(arguments);
                description = $"PNG 생성: {arguments.GetProperty("width").GetInt32()} × {arguments.GetProperty("height").GetInt32()}, RGBA, {asset.Length} bytes";
            }
            else if (name == "copy_asset")
            {
                string source = Resolve(arguments.GetProperty("source").GetString() ?? "");
                CheckAssetExtension(source);
                if (!Path.GetExtension(source).Equals(Path.GetExtension(path), StringComparison.OrdinalIgnoreCase)) throw new InvalidOperationException("원본과 대상의 확장자가 달라서는 안 됩니다.");
                asset = ReadBounded(source, 16 * 1024 * 1024);
                description = "기존 에셋 복사: " + arguments.GetProperty("source").GetString();
            }
            else
            {
                if (pickAsset == null) return "사용자 파일 선택 기능을 사용할 수 없습니다.";
                var chosen = await pickAsset().WaitAsync(token).ConfigureAwait(false);
                if (chosen == null) return "사용자가 가져오기를 취소했습니다.";
                CheckAssetExtension(chosen.Name);
                if (chosen.Bytes.Length > 16 * 1024 * 1024 || !Path.GetExtension(chosen.Name).Equals(Path.GetExtension(path), StringComparison.OrdinalIgnoreCase))
                    throw new InvalidOperationException("선택한 에셋의 크기나 대상 확장자가 맞지 않습니다.");
                asset = chosen.Bytes;
                description = "사용자가 직접 선택한 에셋 가져오기: " + Path.GetFileName(chosen.Name);
            }
            if (confirmChanges && !await approve(relative, "", description).ConfigureAwait(false)) return "에셋 생성을 승인하지 않았습니다.";
            token.ThrowIfCancellationRequested();
            path = ResolveAsset(relative);
            Directory.CreateDirectory(Path.GetDirectoryName(path)!);
            // Complete the bytes before publishing; never truncate an existing asset.
            string staging = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
            try
            {
                await File.WriteAllBytesAsync(staging, asset, token).ConfigureAwait(false);
                token.ThrowIfCancellationRequested();
                File.Move(staging, path, overwrite: false);
            }
            finally { if (File.Exists(staging)) File.Delete(staging); }
            return JsonSerializer.Serialize(new { saved = relative, size = asset.Length, sha256 = Hash(asset),
                note = "디스크에 저장했습니다. 엔진 등록·렌더링·리임포트 성공을 의미하지 않습니다." });
        }
        if (!TextExtensions.Contains(Path.GetExtension(path))) throw new InvalidOperationException("지원하지 않는 프로젝트 텍스트 파일 형식입니다.");
        if (name == "read_project_file")
        {
            byte[] bytes = ReadBounded(path);
            return JsonSerializer.Serialize(new { content = Encoding.UTF8.GetString(bytes).TrimStart('\uFEFF'), sha256 = Hash(bytes) });
        }
        if (name != "write_project_file") throw new InvalidOperationException("지원하지 않는 도구입니다.");
        if (!allowEdits) return "파일 수정 허용이 꺼져 있습니다. 파일을 변경하지 않았습니다.";
        string content = arguments.GetProperty("content").GetString() ?? "";
        byte[] replacement = Encoding.UTF8.GetBytes(content);
        if (replacement.Length > 256 * 1024) throw new InvalidOperationException("한 번에 수정할 수 있는 파일은 256KB까지입니다.");
        string expected = arguments.GetProperty("expectedHash").GetString() ?? "";
        byte[]? original = File.Exists(path) ? ReadBounded(path) : null;
        if (!Matches(original, expected)) return "파일이 변경되었거나 이미 존재합니다. 최신 내용을 다시 읽고 수정해야 합니다.";
        if (confirmChanges && !await approve(relative, original == null ? "" : Encoding.UTF8.GetString(original), content).ConfigureAwait(false))
            return "사용자가 파일 수정을 승인하지 않았습니다. 변경하지 않았습니다.";
        token.ThrowIfCancellationRequested();
        Resolve(relative); // Recheck parent/reparse points after approval.
        byte[]? latest = File.Exists(path) ? ReadBounded(path) : null;
        if (!Matches(latest, expected)) return "승인 중 파일이 바뀌었습니다. 변경하지 않았습니다.";
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        // Do not truncate an existing file until the full replacement has been written.
        string temporary = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
        try
        {
            await File.WriteAllBytesAsync(temporary, replacement, token).ConfigureAwait(false);
            token.ThrowIfCancellationRequested();
            File.Move(temporary, path, original != null);
        }
        finally { if (File.Exists(temporary)) File.Delete(temporary); }
        savedCode?.Invoke(relative, original == null ? "" : Encoding.UTF8.GetString(original).TrimStart('\uFEFF'), content);
        return JsonSerializer.Serialize(new { saved = relative, sha256 = Hash(replacement) });
    }

    private string Resolve(string relative)
    {
        if (Path.IsPathRooted(relative) || relative.Contains(':')) throw new InvalidOperationException("프로젝트 상대 경로만 사용할 수 있습니다.");
        string path = Path.GetFullPath(Path.Combine(_root, relative));
        if (!path.Equals(_root, StringComparison.OrdinalIgnoreCase) && !path.StartsWith(_root + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("프로젝트 밖의 경로는 사용할 수 없습니다.");
        // Reject junctions/symlinks and secret-looking names, including parent segments.
        for (string? current = path; current != null; current = Path.GetDirectoryName(current))
        {
            if (current != _root && Blocked(Path.GetFileName(current))) throw new InvalidOperationException("인증·비밀 파일이나 내부 관리 경로는 접근할 수 없습니다.");
            if ((File.Exists(current) || Directory.Exists(current)) && (File.GetAttributes(current) & FileAttributes.ReparsePoint) != 0)
                throw new InvalidOperationException("심볼릭 링크·정션 경로는 접근할 수 없습니다.");
            if (current.Equals(_root, StringComparison.OrdinalIgnoreCase)) break;
        }
        return path;
    }

    private static bool Blocked(string name) => name.StartsWith('.') || name.Contains("token", StringComparison.OrdinalIgnoreCase)
        || name.Contains("secret", StringComparison.OrdinalIgnoreCase) || name.Contains("credential", StringComparison.OrdinalIgnoreCase)
        || name.Equals("auth.json", StringComparison.OrdinalIgnoreCase) || name.EndsWith(".pem", StringComparison.OrdinalIgnoreCase)
        || name.EndsWith(".key", StringComparison.OrdinalIgnoreCase);
    private string ResolveAsset(string relative)
    {
        string path = Resolve(relative);
        string assetRoot = Path.Combine(_root, "Asset");
        if (!path.StartsWith(assetRoot + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("새 에셋은 Asset 폴더 하위에만 생성할 수 있습니다.");
        return path;
    }
    private static void CheckAssetExtension(string path)
    {
        if (!AssetExtensions.Contains(Path.GetExtension(path))) throw new InvalidOperationException("지원하지 않는 이미지·오디오·폰트 에셋 형식입니다.");
    }
    private static byte[] CreatePng(JsonElement arguments)
    {
        int width = arguments.GetProperty("width").GetInt32(), height = arguments.GetProperty("height").GetInt32();
        if (width < 1 || height < 1 || width > 1024 || height > 1024) throw new InvalidOperationException("PNG 크기는 1~1024 픽셀이어야 합니다.");
        byte[] pixels = new byte[width * height * 4];
        long work = 0;
        void Fill(int x, int y, int w, int h, string color)
        {
            if (x < 0 || y < 0 || w < 1 || h < 1 || (long)x + w > width || (long)y + h > height)
                throw new InvalidOperationException("사각형이 PNG 범위를 벗어났습니다.");
            work += (long)w * h;
            if (work > 16 * 1024 * 1024) throw new InvalidOperationException("PNG 그리기 작업량이 너무 많습니다.");
            if (color.Length != 9 || color[0] != '#') throw new InvalidOperationException("색은 #RRGGBBAA 형식이어야 합니다.");
            byte[] rgba = Convert.FromHexString(color[1..]);
            for (int row = y; row < y + h; row++)
                for (int column = x; column < x + w; column++)
                {
                    int i = (row * width + column) * 4;
                    pixels[i] = rgba[2]; pixels[i + 1] = rgba[1]; pixels[i + 2] = rgba[0]; pixels[i + 3] = rgba[3];
                }
        }
        Fill(0, 0, width, height, arguments.GetProperty("background").GetString() ?? "");
        var rectangles = arguments.GetProperty("rectangles");
        if (rectangles.GetArrayLength() > 4096) throw new InvalidOperationException("사각형은 4096개까지입니다.");
        foreach (var r in rectangles.EnumerateArray()) Fill(r.GetProperty("x").GetInt32(), r.GetProperty("y").GetInt32(),
            r.GetProperty("width").GetInt32(), r.GetProperty("height").GetInt32(), r.GetProperty("color").GetString() ?? "");
        var source = BitmapSource.Create(width, height, 96, 96, PixelFormats.Bgra32, null, pixels, width * 4);
        source.Freeze();
        var encoder = new PngBitmapEncoder();
        encoder.Frames.Add(BitmapFrame.Create(source));
        using var stream = new MemoryStream();
        encoder.Save(stream);
        return stream.ToArray();
    }
    private static byte[] ReadBounded(string path, int limit = 256 * 1024)
    {
        using var file = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
        if (file.Length > limit) throw new InvalidOperationException("파일이 허용 크기를 초과했습니다.");
        byte[] bytes = new byte[checked((int)file.Length)];
        file.ReadExactly(bytes);
        return bytes;
    }
    private static string Hash(byte[] value) => Convert.ToHexString(SHA256.HashData(value));
    private static bool Matches(byte[]? bytes, string expected) => bytes == null ? expected.Length == 0 : Hash(bytes).Equals(expected, StringComparison.OrdinalIgnoreCase);
}
