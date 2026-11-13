using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.Encodings.Web;
using System.Text.Unicode;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace PixelTool;

// No arbitrary shell or network access; Asset file deletion follows the editor approval mode.
internal sealed class ChatGptProjectTools(string project)
{
    private static readonly JsonSerializerOptions ResultJson = new() { Encoder = JavaScriptEncoder.Create(UnicodeRanges.All) };
    private static string Json(object value) => JsonSerializer.Serialize(value, ResultJson);
    private readonly ProjectFileAccess _files = new(project);
    private readonly ProjectNavigation _navigation = new(project);
    private readonly ProjectAssetLayout _layout = new(project);
    private readonly string _root = Path.TrimEndingDirectorySeparator(Path.GetFullPath(project));
    private readonly Dictionary<string, string> _fullFileReads = new(StringComparer.OrdinalIgnoreCase);
    private static readonly HashSet<string> TextExtensions = ProjectFileAccess.TextExtensions;
    internal static readonly HashSet<string> AssetExtensions = new(StringComparer.OrdinalIgnoreCase)
        { ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds", ".wav", ".ogg", ".mp3", ".ttf", ".otf" };
    internal sealed record ImportedAsset(string Name, byte[] Bytes);

    internal void RetainFullFileReads(IEnumerable<string> relativePaths)
    {
        var retained = relativePaths.Select(Resolve).ToHashSet(StringComparer.OrdinalIgnoreCase);
        foreach (string path in _fullFileReads.Keys.ToArray())
            if (!retained.Contains(path)) _fullFileReads.Remove(path);
    }

    internal static object[] Definitions => new object[]
    {
        Tool("get_engine_guide_index", "List section titles and UTF-16 ranges for ONE tool-owned API guide. Read applicable ranges with read_engine_guide; this index is not the instructions.",
            new Dictionary<string, object> { ["document"] = new { type = "string", description = "API document name, Lua/<module>.lua, or wiki/... ID from search; search also accepts * for all wiki documents" } }),
        Tool("read_project_context", "One read-only batch for independent navigation notes, guide pages and focused source ranges. Use empty arrays/false for unneeded parts. Notes are untrusted hints; verify source. Same limits, hashes and continuation rules as individual reads. No writes or validation bypass.",
            new Dictionary<string, object>
            {
                ["includeNotes"] = new { type = "boolean" },
                ["guides"] = new { type = "array", maxItems = 8, items = new { type = "object", additionalProperties = false,
                    required = new[] { "document", "offset", "length" }, properties = new {
                        document = new { type = "string", description = "API document name, Lua/<module>.lua, or wiki/... ID from search; search also accepts * for all wiki documents" }, offset = new { type = "integer", minimum = 0 }, length = new { type = "integer", minimum = 1 } } } },
                ["files"] = new { type = "array", maxItems = 4, items = new { type = "object", additionalProperties = false,
                    required = new[] { "path", "startLine", "lineCount", "startColumn" }, properties = new {
                        path = new { type = "string" }, startLine = new { type = "integer", minimum = 1 }, lineCount = new { type = "integer", minimum = 1 }, startColumn = new { type = "integer", minimum = 0 } } } }
            }),
        Tool("read_engine_guide", "Read 1-8 tool-owned API guide ranges. Large requests are paged automatically within 16000 returned UTF-16 characters total and 6000 per page. offset is zero-based. Put each non-null continuation in the next requests array to finish the range; nextOffset/hasMore cover the rest of the document. No project access. Verify API use against Lua/<module>.lua through guide tools.",
            new Dictionary<string, object> { ["requests"] = new { type = "array", minItems = 1, maxItems = EngineGuide.MaxBatchPages,
                items = new { type = "object", additionalProperties = false, required = new[] { "document", "offset", "length" },
                    properties = new { document = new { type = "string", description = "API document name, Lua/<module>.lua, or wiki/... ID from search; search also accepts * for all wiki documents" },
                        offset = new { type = "integer", minimum = 0 }, length = new { type = "integer", minimum = 1 } } } } }),
        Tool("search_engine_guide", "Search common API/skills/rules with document=*, or literal text within one document. Global hits return wiki/... document IDs, line and snippet; use that ID with read_engine_guide. Up to 20 snippets with UTF-16 offsets/truncation; snippets only locate instructions. Read applicable full sections. Verify API signatures in Lua/<module>.lua through guide tools.",
            new Dictionary<string, object> { ["document"] = new { type = "string", description = "API document name, Lua/<module>.lua, or wiki/... ID from search; search also accepts * for all wiki documents" },
                ["query"] = new { type = "string", minLength = 1, maxLength = 200 }, ["limit"] = new { type = "integer", minimum = 1, maximum = EngineGuide.MaxSearchResults } }),
        Tool("ask_choice", "Ask the user one concrete question with 2 to 5 clickable options. Wait for their selection or custom text before continuing. Use for meaningful choices, not approvals or decisions already clear from project code.",
            new Dictionary<string, object> { ["question"] = new { type = "string", minLength = 1, maxLength = 500 },
                ["options"] = new { type = "array", minItems = 2, maxItems = 5,
                    items = new { type = "string", minLength = 1, maxLength = 150 } } }),
        Tool("reimport_project", "Clear the current log window/history, then reload Asset/main.lua on the render thread, resetting runtime. Refuses unsaved buffers/paused debugger before clearing. Returns after the reload frame with log cursor and new logs. Inspect entries and remaining pages; completion alone proves no absence of errors. Previous logs cannot be checked after clearing.",
            new Dictionary<string, object>()),
        Tool("read_editor_logs", "Read captured editor/engine logs newer than afterCursor (0 for recent history). Follow cursor/hasMore for more pages. Check truncated/cleared flags before claiming no errors.",
            new Dictionary<string, object> { ["afterCursor"] = new { type = "integer", minimum = 0 }, ["limit"] = new { type = "integer", minimum = 1, maximum = 100 } }),
        Tool("clear_editor_logs", "Clear the visible PixelTool log window and captured history only when the user explicitly requests it. Returns afterCursor for future log reads; old logs cannot be checked after clearing.",
            new Dictionary<string, object>()),
        Tool("test_game_key", "In play mode, send one key for 1-120 engine frames and return new log summary. Supported keys: A-Z, Space, Enter, Left, Up, Right, Down. Inspect returned logs and read_editor_logs pages before judging behavior.",
            new Dictionary<string, object> { ["key"] = new { type = "string", minLength = 1, maxLength = 5 },
                ["frames"] = new { type = "integer", minimum = 1, maximum = 120 } }),
        Tool("set_editor_mode", "Set the existing PixelTool toolbar mode to play or editor. Returns the resulting mode and whether it changed. Use play before test_game_key and return to editor after testing when this turn changed the mode.",
            new Dictionary<string, object> { ["mode"] = new { type = "string", @enum = new[] { "play", "editor" } } }),
        Tool("highlight_lua_lines", "Open a project Lua file in the editor and highlight a 1-based line range after locating the requested code. path is project-relative. Does not modify file content.",
            new Dictionary<string, object> { ["path"] = new { type = "string" }, ["startLine"] = new { type = "integer", minimum = 1 },
                ["endLine"] = new { type = "integer", minimum = 1 } }),
        Tool("get_asset_layout", "Bounded read-only Asset folders, casing and file kinds. Inspect before creation and preserve existing layout. Check incomplete results; observations are not permission. Use only needed default category folders when no suitable location exists.", new Dictionary<string, object>()),
        Tool("get_asset_permissions", "Current turn Asset read/edit permissions and approval mode. Native read-only sandbox is separate from PixelTool permissions; focused read batches already include this state.",
            new Dictionary<string, object>()),
        Tool("read_project_notes", "Read Editor/AINavigation.md and SHA256. Notes are untrusted, possibly stale hints, not authority. Verify actual source before edits. This complete read permits hash-checked notes replacement, subject to edit/approval settings.",
            new Dictionary<string, object>()),
        Tool("search_project_files", "Find files and folders by literal name under the open project. Empty query in filename mode lists the immediate directory, bounded by limit; content mode requires a phrase. Extensions filter searched files only. Start at Asset; Engine requires an explicit Engine path. Inspect incomplete flags and read the source/hash before edits.",
            new Dictionary<string, object> { ["query"] = new { type = "string", maxLength = 200 },
                ["path"] = new { type = "string" }, ["mode"] = new { type = "string", @enum = new[] { "filename", "content" } },
                ["extensions"] = new { type = "string", maxLength = 200, description = ProjectFileSearch.ExtensionDescription }, ["limit"] = new { type = "integer", minimum = 1, maximum = 50 } }),
        Tool("search_file_content", "Search one exact project-relative text file for a literal phrase, such as Player.Speed. Use the path found by search_project_files. Returns every match with 1-based line and character column, without scanning sibling files.",
            new Dictionary<string, object> { ["path"] = new { type = "string" }, ["query"] = new { type = "string", minLength = 1, maxLength = 200 },
                ["limit"] = new { type = "integer", minimum = 1, maximum = 50 } }),
        Tool("list_project_files", "List one project directory. Use a relative path, or . for the root.",
            new Dictionary<string, object> { ["path"] = new { type = "string" } }),
        Tool("read_project_file", "Read a project text file and its SHA256 for safe replacement. Paths are relative to the project.",
            new Dictionary<string, object> { ["path"] = new { type = "string" } }),
        Tool("read_project_file_lines", "Read 1-4 focused text ranges. Large requests are paged within 200 returned lines and 12000 content characters total, shared equally. Prefer complete lines; long lines continue at zero-based UTF-16 startColumn (normally 0). Put each non-null continuation in the next requests array to finish the range; compare SHA256 between pages. Returns actual ranges and partial/truncated/hasMore. Strip line numbers; whole-file replacement still requires read_project_file.",
            new Dictionary<string, object> { ["requests"] = new { type = "array", minItems = 1, maxItems = 4,
                items = new { type = "object", additionalProperties = false, required = new[] { "path", "startLine", "lineCount", "startColumn" },
                    properties = new { path = new { type = "string" }, startLine = new { type = "integer", minimum = 1 },
                        lineCount = new { type = "integer", minimum = 1 }, startColumn = new { type = "integer", minimum = 0 } } } } }),
        Tool("write_project_file", "Create or replace one project text file. Read first and supply its expected SHA256; for a new file use an empty expectedHash. A new .pxm uses local Script = {}, Script:Spawn(), Script:Update(DTime), and return Script; never use its filename as the Lua table name. Empty new .pxm content gets the bundled template. Never delete.",
            new Dictionary<string, object> { ["path"] = new { type = "string" }, ["content"] = new { type = "string" }, ["expectedHash"] = new { type = "string" } }),
        Tool("patch_project_file", "Replace one unique exact span, with current whole-file SHA256 from a full or focused read. Rejects stale hashes and absent/repeated oldText. Whole-file reading is unnecessary for this patch.",
            new Dictionary<string, object> { ["path"] = new { type = "string" }, ["expectedHash"] = new { type = "string" },
                ["oldText"] = new { type = "string", minLength = 1 }, ["newText"] = new { type = "string" } }),
        Tool("delete_asset_file", "Move ONE existing file strictly inside Asset/ to the Windows Recycle Bin. Never delete folders, Asset itself, Engine, Editor or outside paths. Follow the editor approval setting: automatic mode needs no additional confirmation. Explain references/impact in reason. No permanent-delete fallback.",
            new Dictionary<string, object> { ["path"] = new { type = "string" }, ["reason"] = new { type = "string", minLength = 1, maxLength = 1000 } }),
        Tool("delete_asset_folder", "Recycle ONE folder and all its contents strictly inside Asset/. Never delete Asset itself or outside paths. Reject links, secret paths and unsaved editor buffers anywhere in the subtree. Follow current edit/approval settings.",
            new Dictionary<string, object> { ["path"] = new { type = "string" }, ["reason"] = new { type = "string", minLength = 1, maxLength = 1000 } }),
        Tool("move_asset", "Move or rename one file OR folder within Asset/. path is source and destination is the complete new relative path. Destination parent must exist. Never overwrite or move a folder into itself. Check and update project references separately. Follow current edit/approval settings.",
            new Dictionary<string, object> { ["path"] = new { type = "string" }, ["destination"] = new { type = "string" },
                ["reason"] = new { type = "string", minLength = 1, maxLength = 1000 } }),
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

    private static object Tool(string name, string description, Dictionary<string, object> properties)
    {
        if (name is "write_project_file" or "create_png_asset" or "copy_asset" or "import_asset")
        {
            properties["allowRootPlacement"] = new { type = "boolean", description = "Normally false. True only for a user's explicit Asset-root destination; it does not expand path or plan scope." };
            description += " Use the user-specified destination. Inspect layout only when a destination must be chosen; a new file does not require reading project notes.";
        }
        return new { type = "function", name, description, strict = true,
            parameters = new { type = "object", properties, required = properties.Keys.ToArray(), additionalProperties = false } };
    }

    internal async Task<string> InvokeAsync(string name, JsonElement arguments, bool allowEdits, bool confirmChanges,
        Func<string, string, string, Task<bool>> approve, CancellationToken token, Func<Task<ImportedAsset?>>? pickAsset = null,
        Action<string, string, string>? savedCode = null, Func<string, Task<bool>>? canDeleteAssetFile = null)
    {
        token.ThrowIfCancellationRequested();
        if (name == "get_engine_guide_index") return EngineGuide.Index(arguments.GetProperty("document").GetString() ?? "");
        if (name == "read_project_context")
        {
            bool notes = arguments.GetProperty("includeNotes").GetBoolean();
            var guides = arguments.GetProperty("guides");
            var files = arguments.GetProperty("files");
            if (guides.ValueKind != JsonValueKind.Array || guides.GetArrayLength() > EngineGuide.MaxBatchPages ||
                files.ValueKind != JsonValueKind.Array || files.GetArrayLength() > 4 ||
                (!notes && guides.GetArrayLength() == 0 && files.GetArrayLength() == 0))
                throw new ArgumentException("Select notes, up to 8 guide pages and/or up to 4 source ranges.");
            foreach (var range in files.EnumerateArray()) ValidateReadRange(range);
            var results = new Dictionary<string, JsonElement>();
            async Task Add(string key, string tool, object args)
            {
                using var doc = JsonDocument.Parse(await InvokeAsync(tool, JsonSerializer.SerializeToElement(args),
                    allowEdits, confirmChanges, approve, token).ConfigureAwait(false));
                results[key] = doc.RootElement.Clone();
            }
            if (guides.GetArrayLength() > 0) await Add("guides", "read_engine_guide", new { requests = guides });
            if (files.GetArrayLength() > 0) await Add("files", "read_project_file_lines", new { requests = files });
            if (notes) await Add("notes", "read_project_notes", new { });
            return Json(new { results, canRead = true, canEdit = allowEdits,
                approvalMode = confirmChanges ? "alwaysConfirm" : "automatic" });
        }
        if (name == "read_engine_guide")
        {
            if (arguments.TryGetProperty("requests", out var pages)) return EngineGuide.ReadBatch(pages);
            return EngineGuide.Read(arguments.GetProperty("document").GetString() ?? "",
                arguments.GetProperty("offset").GetInt32(), arguments.GetProperty("length").GetInt32());
        }
        if (name == "read_project_file_lines" && arguments.TryGetProperty("requests", out var ranges))
        {
            if (ranges.ValueKind != JsonValueKind.Array || ranges.GetArrayLength() is < 1 or > 4)
                throw new ArgumentException("Use 1 to 4 source ranges.");
            // Validate every path before reading; size alone is handled by bounded paging.
            foreach (var range in ranges.EnumerateArray()) ValidateReadRange(range);
            var files = new List<JsonElement>();
            foreach (var range in ranges.EnumerateArray())
            {
                token.ThrowIfCancellationRequested();
                using var result = JsonDocument.Parse(ReadFileLines(range, 12000 / ranges.GetArrayLength(), 200 / ranges.GetArrayLength()));
                files.Add(result.RootElement.Clone());
            }
            return Json(new { files, canRead = true, canEdit = allowEdits,
                approvalMode = confirmChanges ? "alwaysConfirm" : "automatic" });
        }
        if (name == "search_engine_guide")
            return await Task.Run(() => EngineGuide.Search(arguments.GetProperty("document").GetString() ?? "",
                arguments.GetProperty("query").GetString() ?? "", arguments.GetProperty("limit").GetInt32(), token), token).ConfigureAwait(false);
        if (name == "get_asset_layout") return _layout.GetInfo();
        if (name == "get_asset_permissions")
            return Json(new { assetRoot = "Asset/", canRead = true, canEdit = allowEdits,
                approvalMode = confirmChanges ? "alwaysConfirm" : "automatic" });
        if (name == "read_project_notes")
        {
            string result = _navigation.ReadNotes();
            using var document = JsonDocument.Parse(result);
            if (document.RootElement.GetProperty("exists").GetBoolean())
                _fullFileReads[Resolve(ProjectNavigation.NotesPath)] = document.RootElement.GetProperty("sha256").GetString()!;
            return result;
        }
        if (name == "search_project_files")
        {
            string query = arguments.GetProperty("query").GetString() ?? "";
            string scope = arguments.GetProperty("path").GetString() ?? "Asset";
            string mode = arguments.GetProperty("mode").GetString() ?? "filename";
            string extensions = arguments.GetProperty("extensions").GetString() ?? "";
            int limit = arguments.GetProperty("limit").GetInt32();
            if (string.IsNullOrWhiteSpace(query) && mode == "filename") return _navigation.ListDirectory(scope, limit);
            if (string.IsNullOrWhiteSpace(query)) return Json(new { success = false, message = "내용 검색에는 검색어가 필요합니다. 파일을 읽으려면 read_project_file_lines를 사용하세요." });
            return await Task.Run(() => _navigation.Search(query, scope, mode, extensions, limit, token), token).ConfigureAwait(false);
        }
        if (name == "search_file_content")
        {
            string query = arguments.GetProperty("query").GetString() ?? "";
            string file = arguments.GetProperty("path").GetString() ?? "";
            int limit = arguments.GetProperty("limit").GetInt32();
            if (string.IsNullOrWhiteSpace(query)) return Json(new { success = false, message = "파일 내용 검색에는 검색어가 필요합니다. 파일을 읽으려면 read_project_file_lines를 사용하세요." });
            return await Task.Run(() => _navigation.SearchFileContent(file, query, limit, token), token).ConfigureAwait(false);
        }
        string relative = arguments.GetProperty("path").GetString() ?? "";
        string path = Resolve(relative);
        // Explicit Asset-root placement is separate from edit permission.
        if (allowEdits && (name is "write_project_file" or "create_png_asset" or "copy_asset" or "import_asset"))
        {
            string? placement = _layout.GetCreationIssue(relative);
            if (placement != null)
            {
                if (!arguments.TryGetProperty("allowRootPlacement", out var exception) || exception.ValueKind != JsonValueKind.True)
                    throw new InvalidOperationException(placement + " 기본 분류 경로로 다시 계획하세요. 사용자가 루트 경로를 명시한 경우에만 allowRootPlacement=true를 사용하세요.");
            }
        }
        if (name is "delete_asset_file" or "delete_asset_folder" or "move_asset")
        {
            if (!allowEdits) return Json(new { success = false, message = "파일 수정 허용이 꺼져 있습니다. 변경하지 않았습니다." });
            path = ResolveAsset(relative);
            bool directory = Directory.Exists(path);
            if ((!directory && !File.Exists(path)) || (name == "delete_asset_file" && directory) ||
                (name == "delete_asset_folder" && !directory))
                throw new InvalidOperationException("Asset 하위의 해당 종류 파일 또는 폴더가 필요합니다.");
            string? destinationRelative = name == "move_asset" ? arguments.GetProperty("destination").GetString() : null;
            string? destination = destinationRelative == null ? null : ResolveAsset(destinationRelative);
            void CheckDestination()
            {
                if (destination == null) return;
                ResolveAsset(destinationRelative!);
                if (File.Exists(destination) || Directory.Exists(destination))
                    throw new InvalidOperationException("대상이 이미 존재합니다. 덮어쓰지 않습니다.");
                if (!Directory.Exists(Path.GetDirectoryName(destination)))
                    throw new InvalidOperationException("대상 부모 폴더를 먼저 생성해야 합니다.");
                if (directory && AssetFileOperations.IsInside(destination, path))
                    throw new InvalidOperationException("폴더를 자기 내부로 이동할 수 없습니다.");
            }
            CheckDestination();
            string reason = arguments.GetProperty("reason").GetString() ?? "";
            if (string.IsNullOrWhiteSpace(reason) || reason.Length > 1000)
                throw new InvalidOperationException("변경 이유와 영향을 1000자 이내로 설명해야 합니다.");
            if (canDeleteAssetFile == null || !await canDeleteAssetFile(relative).ConfigureAwait(false))
                return Json(new { success = false, message = "저장되지 않은 편집 내용 또는 에디터 상태 때문에 삭제하지 않았습니다." });
            string expectedHash = await HashAssetPathAsync(relative, token).ConfigureAwait(false);
            string action = destination == null ? "휴지통 삭제" : "이동·이름 변경 → " + destinationRelative;
            string details = $"{action}: {relative}\n하위 내용 포함: {directory}\n이유·영향: {reason}\nSHA256: {expectedHash}";
            if (confirmChanges && !await approve(relative, details, action).ConfigureAwait(false))
                return Json(new { success = false, message = "사용자가 변경을 승인하지 않았습니다. 변경하지 않았습니다." });
            token.ThrowIfCancellationRequested();
            path = ResolveAsset(relative);
            if (Directory.Exists(path) != directory || (!directory && !File.Exists(path)) ||
                await HashAssetPathAsync(relative, token).ConfigureAwait(false) != expectedHash)
                return Json(new { success = false, message = "승인 중 대상 또는 하위 내용이 변경되었습니다. 변경하지 않았습니다." });
            CheckDestination();
            token.ThrowIfCancellationRequested();
            ResolveAsset(relative);
            if (!await canDeleteAssetFile(relative).ConfigureAwait(false))
                return Json(new { success = false, message = "에디터 편집 상태가 변경되어 삭제하지 않았습니다." });
            token.ThrowIfCancellationRequested();
            ResolveAsset(relative);
            CheckDestination();
            if (destination != null)
            {
                if (directory) Directory.Move(path, destination);
                else File.Move(path, destination, overwrite: false);
                return Json(new { moved = relative, destination = destinationRelative,
                    note = "Asset 내부로 이동했습니다. 임포트·스크립트·리소스 참조를 확인하세요." });
            }
            if (directory)
                Microsoft.VisualBasic.FileIO.FileSystem.DeleteDirectory(path,
                    Microsoft.VisualBasic.FileIO.UIOption.OnlyErrorDialogs,
                    Microsoft.VisualBasic.FileIO.RecycleOption.SendToRecycleBin,
                    Microsoft.VisualBasic.FileIO.UICancelOption.ThrowException);
            else Microsoft.VisualBasic.FileIO.FileSystem.DeleteFile(path,
                Microsoft.VisualBasic.FileIO.UIOption.OnlyErrorDialogs,
                Microsoft.VisualBasic.FileIO.RecycleOption.SendToRecycleBin,
                Microsoft.VisualBasic.FileIO.UICancelOption.ThrowException);
            if (File.Exists(path) || Directory.Exists(path)) throw new IOException("대상이 남아 있어 삭제 성공으로 처리하지 않습니다.");
            return Json(new { deleted = relative, recycleBin = true,
                note = "Windows 휴지통으로 이동했습니다. 참조를 정리하고 필요한 경우 리임포트하세요." });
        }
        if (name == "list_project_files")
        {
            var entries = Directory.EnumerateFileSystemEntries(path)
                .Where(p => !Blocked(Path.GetFileName(p)) && (File.GetAttributes(p) & FileAttributes.ReparsePoint) == 0)
                .Take(250).Select(p => new { name = Path.GetFileName(p), directory = Directory.Exists(p) }).ToArray();
            return Json(new { entries, limit = 250 });
        }
        if (name == "inspect_asset")
        {
            CheckAssetExtension(path);
            byte[] bytes = ReadBounded(path, 16 * 1024 * 1024);
            return Json(new { path = relative, extension = Path.GetExtension(path), size = bytes.Length, sha256 = Hash(bytes) });
        }
        if (name is "create_asset_folder" or "copy_asset" or "import_asset" or "create_png_asset")
        {
            if (!allowEdits) return Json(new { success = false, message = "파일 수정 허용이 꺼져 있습니다. 에셋을 생성하지 않았습니다." });
            ResolveAsset(relative);
            if (name == "create_asset_folder")
            {
                if (Directory.Exists(path)) return Json(new { success = false, message = "폴더가 이미 존재합니다. 변경하지 않았습니다." });
                if (confirmChanges && !await approve(relative, "", "에셋 폴더 생성").ConfigureAwait(false)) return Json(new { success = false, message = "폴더 생성을 승인하지 않았습니다." });
                token.ThrowIfCancellationRequested();
                Directory.CreateDirectory(ResolveAsset(relative));
                return Json(new { created = relative });
            }
            if (File.Exists(path) || Directory.Exists(path)) return Json(new { success = false, message = "대상이 이미 존재합니다. 에셋은 자동으로 덮어쓰지 않습니다." });
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
                if (pickAsset == null) return Json(new { success = false, message = "사용자 파일 선택 기능을 사용할 수 없습니다." });
                var chosen = await pickAsset().WaitAsync(token).ConfigureAwait(false);
                if (chosen == null) return Json(new { success = false, message = "사용자가 가져오기를 취소했습니다." });
                CheckAssetExtension(chosen.Name);
                if (chosen.Bytes.Length > 16 * 1024 * 1024 || !Path.GetExtension(chosen.Name).Equals(Path.GetExtension(path), StringComparison.OrdinalIgnoreCase))
                    throw new InvalidOperationException("선택한 에셋의 크기나 대상 확장자가 맞지 않습니다.");
                asset = chosen.Bytes;
                description = "사용자가 직접 선택한 에셋 가져오기: " + Path.GetFileName(chosen.Name);
            }
            if (confirmChanges && !await approve(relative, "", description).ConfigureAwait(false)) return Json(new { success = false, message = "에셋 생성을 승인하지 않았습니다." });
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
            return Json(new { saved = relative, size = asset.Length, sha256 = Hash(asset),
                note = "디스크에 저장했습니다. 엔진 등록·렌더링·리임포트 성공을 의미하지 않습니다." });
        }
        if (name == "read_project_file_lines") return ReadFileLines(arguments);
        if (!TextExtensions.Contains(Path.GetExtension(path))) throw new InvalidOperationException("지원하지 않는 프로젝트 텍스트 파일 형식입니다.");
        if (name == "read_project_file")
        {
            byte[] bytes = ReadBounded(path);
            string text = Encoding.UTF8.GetString(bytes).TrimStart('\uFEFF');
            _fullFileReads[path] = Hash(bytes);
            return Json(new { path = relative, content = text, sha256 = Hash(bytes), partial = false });
        }
        if (name == "patch_project_file")
        {
            RequireWritableTextPath(path);
            return await PatchProjectFileAsync(relative, path, arguments, allowEdits, confirmChanges,
                approve, token, savedCode).ConfigureAwait(false);
        }
        if (name != "write_project_file") throw new InvalidOperationException("지원하지 않는 도구입니다.");
        RequireWritableTextPath(path);
        if (!allowEdits) return Json(new { success = false, message = "파일 수정 허용이 꺼져 있습니다. 파일을 변경하지 않았습니다." });
        string content = arguments.GetProperty("content").GetString() ?? "";
        if (!File.Exists(path) && Path.GetExtension(path).Equals(".pxm", StringComparison.OrdinalIgnoreCase))
            content = LuaFileManager.PrepareNewPxmContent(content);
        byte[] replacement = Encoding.UTF8.GetBytes(content);
        ProjectNavigation.CheckNotesSize(Path.GetRelativePath(_root, path), replacement.Length);
        if (replacement.Length > 256 * 1024) throw new InvalidOperationException("한 번에 수정할 수 있는 파일은 256KB까지입니다.");
        string expected = arguments.GetProperty("expectedHash").GetString() ?? "";
        byte[]? original = File.Exists(path) ? ReadBounded(path) : null;
        if (!Matches(original, expected)) return Json(new { success = false, message = "파일이 변경되었거나 이미 존재합니다. 최신 내용을 다시 읽고 수정해야 합니다." });
        if (original != null && (!_fullFileReads.TryGetValue(path, out var readHash) || !readHash.Equals(expected, StringComparison.OrdinalIgnoreCase)))
            return Json(new { success = false, message = "부분 조회만으로 전체 파일을 교체할 수 없습니다. read_project_file로 현재 원문 전체를 먼저 읽으세요." });
        if (confirmChanges && !await approve(relative, original == null ? "" : Encoding.UTF8.GetString(original), content).ConfigureAwait(false))
            return Json(new { success = false, message = "사용자가 파일 수정을 승인하지 않았습니다. 변경하지 않았습니다." });
        token.ThrowIfCancellationRequested();
        Resolve(relative); // Recheck parent/reparse points after approval.
        byte[]? latest = File.Exists(path) ? ReadBounded(path) : null;
        if (!Matches(latest, expected)) return Json(new { success = false, message = "승인 중 파일이 바뀌었습니다. 변경하지 않았습니다." });
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
        _fullFileReads[path] = Hash(replacement);
        return Json(new { saved = relative, sha256 = Hash(replacement) });
    }

    private (string Relative, string Path, int StartLine, int LineCount, int StartColumn) ValidateReadRange(JsonElement arguments)
    {
        string relative = arguments.GetProperty("path").GetString() ?? "";
        string path = Resolve(relative);
        if (!TextExtensions.Contains(Path.GetExtension(path)))
            throw new ArgumentException($"Unsupported text extension for '{relative}'. Use a supported project text file, such as .lua, .pxm, .scene, .json, .md or .txt; use inspect_asset for media.");
        int startLine = arguments.GetProperty("startLine").GetInt32();
        int lineCount = arguments.GetProperty("lineCount").GetInt32();
        int startColumn = arguments.TryGetProperty("startColumn", out var column) ? column.GetInt32() : 0;
        if (startLine < 1) throw new ArgumentOutOfRangeException(nameof(startLine), $"'{relative}': startLine is 1-based and must be at least 1.");
        if (lineCount < 1) throw new ArgumentOutOfRangeException(nameof(lineCount), $"'{relative}': lineCount must be positive; large ranges are paged automatically.");
        if (startColumn < 0) throw new ArgumentOutOfRangeException(nameof(startColumn), $"'{relative}': startColumn must be nonnegative; use 0 or the returned continuation.");
        return (relative, path, startLine, lineCount, startColumn);
    }

    private string ReadFileLines(JsonElement arguments, int contentLimit = 12000, int lineLimit = 200)
    {
        var (relative, path, startLine, lineCount, startColumn) = ValidateReadRange(arguments);
        byte[] bytes = ReadBounded(path);
        string text = Encoding.UTF8.GetString(bytes).TrimStart('\uFEFF');
        string[] lines = text.Replace("\r\n", "\n").Replace('\r', '\n').Split('\n');
        if (startLine > lines.Length)
            throw new ArgumentOutOfRangeException(nameof(startLine), $"'{relative}' has {lines.Length} lines; startLine must be between 1 and {lines.Length}.");
        string first = lines[startLine - 1];
        if (startColumn > first.Length)
            throw new ArgumentOutOfRangeException(nameof(startColumn), $"'{relative}': line {startLine} has {first.Length} UTF-16 characters; use a column within that line.");
        if (startColumn > 0 && startColumn < first.Length && char.IsLowSurrogate(first[startColumn]) && char.IsHighSurrogate(first[startColumn - 1]))
            throw new ArgumentException($"'{relative}': startColumn splits a Unicode character. Use the returned continuation.");
        int requestedEnd = startLine - 1 + Math.Min(lineCount, lines.Length - startLine + 1);
        int limit = startLine - 1 + Math.Min(lineLimit, requestedEnd - startLine + 1);
        int nextIndex = startLine - 1, nextColumn = startColumn, endLine = startLine - 1;
        var selected = new StringBuilder();
        while (nextIndex < limit)
        {
            string line = lines[nextIndex], prefix = $"{nextIndex + 1}: ";
            int remaining = contentLimit - selected.Length;
            int count = line.Length - nextColumn;
            if (prefix.Length + count + 1 > remaining)
            {
                // Stop before an incomplete line, except when that line itself exceeds the page.
                if (selected.Length > 0) break;
                count = Math.Min(count, remaining - prefix.Length - 1);
                if (count > 0 && nextColumn + count < line.Length && char.IsHighSurrogate(line[nextColumn + count - 1]) && char.IsLowSurrogate(line[nextColumn + count])) count--;
                selected.Append(prefix).Append(line.AsSpan(nextColumn, count)).Append('\n');
                nextColumn += count;
                endLine = nextIndex + 1;
                break;
            }
            selected.Append(prefix).Append(line.AsSpan(nextColumn)).Append('\n');
            endLine = ++nextIndex;
            nextColumn = 0;
        }
        int remainingLineCount = requestedEnd - nextIndex;
        bool truncated = remainingLineCount > 0;
        return Json(new { path = relative, content = selected.ToString(), sha256 = Hash(bytes),
            startLine, startColumn, endLine, totalLines = lines.Length, requestedLineCount = lineCount,
            returnedLineCount = endLine - startLine + 1, partial = true, truncated,
            nextStartLine = nextIndex + 1, nextStartColumn = nextColumn, endsMidLine = nextColumn > 0,
            hasMore = nextIndex < lines.Length, remainingLineCount,
            continuation = truncated ? new { path = relative, startLine = nextIndex + 1, lineCount = remainingLineCount, startColumn = nextColumn } : null });
    }

    private async Task<string> PatchProjectFileAsync(string relative, string path, JsonElement arguments, bool allowEdits,
        bool confirmChanges, Func<string, string, string, Task<bool>> approve, CancellationToken token,
        Action<string, string, string>? savedCode)
    {
        if (!allowEdits) return Json(new { success = false, message = "파일 수정이 꺼져 있습니다." });
        string expected = arguments.GetProperty("expectedHash").GetString() ?? "";
        string oldText = arguments.GetProperty("oldText").GetString() ?? "";
        string newText = arguments.GetProperty("newText").GetString() ?? "";
        if (oldText.Length == 0 || oldText == newText) throw new InvalidOperationException("서로 다른 원문과 수정문이 필요합니다.");
        byte[] original = ReadBounded(path);
        if (!Matches(original, expected)) return Json(new { success = false, message = "파일이 변경되었습니다. 최신 내용을 다시 읽고 수정하세요." });
        string before = Encoding.UTF8.GetString(original);
        int offset = before.IndexOf(oldText, StringComparison.Ordinal);
        if (offset < 0 || before.IndexOf(oldText, offset + 1, StringComparison.Ordinal) >= 0)
            return Json(new { success = false, message = "원문이 없거나 두 번 이상 나타납니다. 수정하지 않았습니다." });
        string after = before[..offset] + newText + before[(offset + oldText.Length)..];
        byte[] replacement = Encoding.UTF8.GetBytes(after);
        ProjectNavigation.CheckNotesSize(Path.GetRelativePath(_root, path), replacement.Length);
        if (replacement.Length > 256 * 1024) throw new InvalidOperationException("수정 결과는 256KB까지입니다.");
        if (confirmChanges && !await approve(relative, before, after).ConfigureAwait(false)) return Json(new { success = false, message = "사용자가 수정을 승인하지 않았습니다." });
        token.ThrowIfCancellationRequested();
        Resolve(relative);
        byte[] latest = ReadBounded(path);
        if (!Matches(latest, expected)) return Json(new { success = false, message = "승인 중 파일이 변경되었습니다. 수정하지 않았습니다." });
        string temporary = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
        try
        {
            await File.WriteAllBytesAsync(temporary, replacement, token).ConfigureAwait(false);
            token.ThrowIfCancellationRequested();
            File.Move(temporary, path, overwrite: true);
        }
        finally { if (File.Exists(temporary)) File.Delete(temporary); }
        savedCode?.Invoke(relative, before.TrimStart('\uFEFF'), after.TrimStart('\uFEFF'));
        return Json(new { saved = relative, sha256 = Hash(replacement), previousSha256 = expected,
            exactPatch = new { occurrences = 1, replacedCharacters = oldText.Length, insertedCharacters = newText.Length, unrelatedTextPreserved = true } });
    }

    private string Resolve(string relative) => _files.Resolve(relative);
    private void RequireWritableTextPath(string path)
    {
        string relative = Path.GetRelativePath(_root, path).Replace('\\', '/');
        if (!relative.StartsWith("Asset/", StringComparison.OrdinalIgnoreCase) &&
            !relative.Equals(ProjectNavigation.NotesPath, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException(ProjectFileAccess.AssetMutationDenied);
    }
    private static bool Blocked(string name) => ProjectFileAccess.Blocked(name);

    private string ResolveAsset(string relative)
    {
        string path = Path.TrimEndingDirectorySeparator(Resolve(relative));
        string assetRoot = Path.Combine(_root, "Asset");
        if (!path.StartsWith(assetRoot + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException(ProjectFileAccess.AssetMutationDenied);
        return path;
    }
    private async Task<string> HashAssetPathAsync(string relative, CancellationToken token)
    {
        string path = ResolveAsset(relative);
        if (File.Exists(path)) return "file:" + await HashFileAsync(path, token).ConfigureAwait(false);
        if (!Directory.Exists(path)) throw new DirectoryNotFoundException("대상 폴더가 없습니다.");
        using var hash = IncrementalHash.CreateHash(HashAlgorithmName.SHA256);
        var pending = new Stack<string>();
        pending.Push(path);
        while (pending.Count > 0)
        {
            token.ThrowIfCancellationRequested();
            string folder = pending.Pop();
            ResolveAsset(Path.GetRelativePath(_root, folder));
            hash.AppendData(Encoding.UTF8.GetBytes("directory:" + Path.GetRelativePath(path, folder) + "\n"));
            foreach (string entry in Directory.EnumerateFileSystemEntries(folder).OrderBy(entry => entry, StringComparer.Ordinal))
            {
                token.ThrowIfCancellationRequested();
                ResolveAsset(Path.GetRelativePath(_root, entry));
                if (Directory.Exists(entry)) pending.Push(entry);
                else hash.AppendData(Encoding.UTF8.GetBytes(Path.GetRelativePath(path, entry) + "\0" +
                    await HashFileAsync(entry, token).ConfigureAwait(false) + "\n"));
            }
        }
        return "directory:" + Convert.ToHexString(hash.GetHashAndReset());
    }
    private static async Task<string> HashFileAsync(string path, CancellationToken token)
    {
        using var file = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read,
            81920, FileOptions.Asynchronous | FileOptions.SequentialScan);
        return Convert.ToHexString(await SHA256.HashDataAsync(file, token).ConfigureAwait(false));
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
    private static byte[] ReadBounded(string path, int limit = 256 * 1024) => ProjectFileAccess.ReadBounded(path, limit);
    private static string Hash(byte[] value) => Convert.ToHexString(SHA256.HashData(value));
    private static bool Matches(byte[]? bytes, string expected) => bytes == null ? expected.Length == 0 : Hash(bytes).Equals(expected, StringComparison.OrdinalIgnoreCase);
}
