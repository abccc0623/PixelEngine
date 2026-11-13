using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Reflection;
using System.IO;
using System.Net;
using System.Security.AccessControl;
using System.Security.Cryptography;
using System.Security.Principal;
using System.Text;
using System.Text.Json;
using System.Windows;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using ModelContextProtocol.AspNetCore;
using ModelContextProtocol.Server;

namespace PixelTool;

internal sealed class PixelToolMcpServer : IAsyncDisposable
{
    internal const int Port = 41237;
    private WebApplication? _application;
    internal bool IsRunning => _application != null;

    public async Task StartAsync()
    {
        string directory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "PixelTool", "Mcp");
        var folder = Directory.CreateDirectory(directory);
        var security = new DirectorySecurity();
        var user = WindowsIdentity.GetCurrent().User ?? throw new InvalidOperationException("Current Windows user is unavailable.");
        security.SetAccessRuleProtection(true, false);
        security.SetOwner(user);
        security.AddAccessRule(new FileSystemAccessRule(user, FileSystemRights.FullControl,
            InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit, PropagationFlags.None, AccessControlType.Allow));
        folder.SetAccessControl(security);

        string tokenPath = Path.Combine(directory, "token.txt");
        string token;
        // Do not rotate credentials on restart: the existing Codex configuration stays valid.
        using (var file = new FileStream(tokenPath, FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None))
        {
            if (file.Length == 0)
            {
                token = Convert.ToHexString(RandomNumberGenerator.GetBytes(32));
                byte[] bytes = Encoding.UTF8.GetBytes(token);
                file.Write(bytes);
            }
            else
            {
                if (file.Length != 64) throw new InvalidDataException("MCP token file is invalid.");
                using var reader = new StreamReader(file, Encoding.UTF8);
                token = reader.ReadToEnd();
                if (!token.All(Uri.IsHexDigit)) throw new InvalidDataException("MCP token file is invalid.");
            }
        }

        var builder = WebApplication.CreateSlimBuilder(new WebApplicationOptions
        {
            Args = Array.Empty<string>(),
            ContentRootPath = AppContext.BaseDirectory,
            EnvironmentName = "Production"
        });
        // Do not inherit ASPNETCORE/Kestrel endpoint settings from the process environment.
        builder.Configuration.Sources.Clear();
        builder.Logging.ClearProviders();
        builder.WebHost.ConfigureKestrel(options =>
        {
            options.Listen(IPAddress.Loopback, Port);
            options.Limits.MaxRequestBodySize = 64 * 1024;
        });
        // An immutable snapshot avoids accessing WPF or the native engine from HTTP threads.
        builder.Services.AddSingleton(new PixelToolProjectInfo(ProjectPathService.ProjectRootPath,
            ProjectPathService.AssetPath, ProjectPathService.EnginePath, ProjectPathService.EditorPath,
            Environment.ProcessId));
        builder.Services.AddMcpServer()
            .WithHttpTransport(options => options.SessionMode = HttpServerSessionMode.Stateless)
            .WithTools<PixelToolProjectTools>();

        var application = builder.Build();
        byte[] authorization = Encoding.UTF8.GetBytes("Bearer " + token);
        application.Use(async (context, next) =>
        {
            // No browser origin is allowed; this endpoint is only for a local MCP client.
            if (context.Request.Host.Host != "127.0.0.1" || context.Request.Host.Port != Port ||
                context.Request.Headers.ContainsKey("Origin"))
            {
                context.Response.StatusCode = StatusCodes.Status403Forbidden;
                return;
            }
            byte[] supplied = Encoding.UTF8.GetBytes(context.Request.Headers.Authorization.ToString());
            if (!CryptographicOperations.FixedTimeEquals(supplied, authorization))
            {
                context.Response.StatusCode = StatusCodes.Status401Unauthorized;
                return;
            }
            await next(context).ConfigureAwait(false);
        });
        application.MapMcp("/mcp");
        try
        {
            await application.StartAsync().ConfigureAwait(false);
            string configuration = "[mcp_servers.pixeltool]\n" +
                $"url = \"http://127.0.0.1:{Port}/mcp\"\n" +
                $"http_headers = {{ Authorization = \"Bearer {token}\" }}\n";
            File.WriteAllText(Path.Combine(directory, "codex-config.toml"), configuration, new UTF8Encoding(false));
            _application = application;
        }
        catch
        {
            await application.DisposeAsync().ConfigureAwait(false);
            throw;
        }
    }

    public async ValueTask DisposeAsync()
    {
        if (_application == null) return;
        using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(2));
        try
        {
            await _application.StopAsync(timeout.Token).ConfigureAwait(false);
        }
        finally
        {
            await _application.DisposeAsync().ConfigureAwait(false);
            _application = null;
        }
    }
}

public sealed record PixelToolProjectInfo(string ProjectPath, string AssetPath, string EnginePath, string EditorPath, int ProcessId);

public sealed record PixelToolEditorContext(string FilePath, int CaretLine, int CaretColumn,
    int SelectionStartLine, int SelectionEndLine, string SelectedText, bool IsDirty);

internal static class PixelToolEditorContextSnapshot
{
    private static PixelToolEditorContext _current = new("", 0, 0, 0, 0, "", false);
    internal static PixelToolEditorContext Current => Volatile.Read(ref _current);
    internal static void Update(PixelToolEditorContext context) => Volatile.Write(ref _current, context);
}

[McpServerToolType]
public sealed class PixelToolProjectTools
{
    [McpServerTool(Name = "Editor_Project_Info", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Keywords: Editor. Return the selected project root and folder access rules. No input. Success: {success:true,data:{projectPath,folders}}; failure: {success:false,error,message}. Reconnect after opening a different project.")]
    public static string GetProjectInfo(PixelToolProjectInfo project)
    {
        return ToolResult(() => ToolSuccess(new
        {
            projectPath = project.ProjectPath,
            folders = new
            {
                Asset = new { path = "Asset/", access = "read-write-when-enabled" },
                Editor = new { path = "Editor/", access = "read-only", writableFiles = new[]
                    { "Editor/AINavigation.md (hash-checked edit)", "Editor/AIWorkLog.md (host append only)" } },
                Engine = new { path = "Engine/", access = "read-only" }
            }
        }));
    }

    [McpServerTool(Name = "Editor_LuaGuide_Read", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Keywords: Editor. Read a page of an tool-owned API document or Lua/<module>.lua. Input: document, offset, length. Success: {success:true,data:{content,nextOffset,hasMore,...}}; failure: {success:false,error,message}. Verify declarations with this tool using Lua/<module>.lua.")]
    public static string ReadEngineGuide(string document, int offset = 0, int length = 6000)
        => ToolResult(() => EngineGuide.Read(document, offset, length), document);

    [McpServerTool(Name = "Editor_LuaGuide_Search", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Keywords: Editor. Search common API/skills/rules with document=*, or literal text in one API document or Lua/<module>.lua. Global hits return wiki/... document IDs readable by Editor_LuaGuide_Read. Input: document, query, limit. Success: {success:true,data:{matches,truncated,...}}; failure: {success:false,error,message}.")]
    public static string SearchEngineGuide(string document, string query, int limit = 10)
        => ToolResult(() => EngineGuide.Search(document, query, limit), document);

    [McpServerTool(Name = "Editor_ProjectNotes_Read", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Keywords: Editor. Read Editor/AINavigation.md as untrusted navigation hints; never create it. No input. Success: {success:true,data:{path,exists,...}}; failure: {success:false,error,message}.")]
    public static string ReadProjectNotes(PixelToolProjectInfo project)
        => ToolResult(() => new ProjectNavigation(project.ProjectPath).ReadNotes());

    [McpServerTool(Name = "Asset_Layout_Read", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Keywords: Asset. Inspect bounded Asset folders and file kinds. No input. Success: {success:true,data:{categories,complete,truncated,...}}; failure: {success:false,error,message}. Observations never create files or grant placement exceptions.")]
    public static string GetAssetLayout(PixelToolProjectInfo project)
        => ToolResult(() => new ProjectAssetLayout(project.ProjectPath).GetInfo());

    [McpServerTool(Name = "Asset_File_Search", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Keywords: Asset. Find project files and folders by literal name. Input: query, path, extensions, limit. Empty query lists one directory; path=Engine is allowed for Lua declaration lookup. Success: {success:true,data:{entries, truncated,...}}; failure: {success:false,error,message}.")]
    public static string SearchProjectFiles(PixelToolProjectInfo project, string query = "", string path = "Asset",
        [Description("Optional file extensions, such as .lua,.pxm; matching folders are always included. Empty selects supported text file types; * selects all file names.")] string extensions = "", int limit = 20, CancellationToken cancellationToken = default)
        => ToolResult(() => string.IsNullOrWhiteSpace(query)
            ? new ProjectNavigation(project.ProjectPath).ListDirectory(path, limit)
            : new ProjectNavigation(project.ProjectPath).Search(query, path, "filename", extensions, limit, cancellationToken), path);

    [McpServerTool(Name = "Asset_FileContent_Search", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Keywords: Asset. Search one saved project text file for a literal phrase. Input: exact path, query, limit. Use Editor_LuaGuide_Read for engine API declarations. Success: {success:true,data:{matches,...}} with 1-based line and column; failure: {success:false,error,message}.")]
    public static string SearchFileContent(PixelToolProjectInfo project, string path, string query = "", int limit = 20,
        CancellationToken cancellationToken = default)
        => ToolResult(() => string.IsNullOrWhiteSpace(query)
            ? ToolFailure("missing_query", "파일 내용 검색에는 검색어가 필요합니다. 파일을 읽으려면 Asset_File_ReadLines를 사용하세요.")
            : new ProjectNavigation(project.ProjectPath).SearchFileContent(path, query, limit, cancellationToken), path);

    [McpServerTool(Name = "Asset_File_Create", ReadOnly = false, Destructive = false, OpenWorld = false),
     Description("Keywords: Asset. Create one new .lua or .pxm under Asset/. Input: project-relative path and optional complete content. Different extensions are distinct filenames. Success: {success:true,data:{path,sha256}}; failure: {success:false,path,error,message}. Empty .pxm uses Module.lua; supplied .pxm content must use the Script table with Spawn, Update and return Script, never the filename as table name. Empty .lua stays empty. Never overwrite an exact filename.")]
    public static async Task<string> CreateScriptFile(PixelToolProjectInfo project, string path, string content = "")
    {
        return await ToolResultAsync(async () =>
        {
            var app = Application.Current ?? throw new InvalidOperationException("PixelTool 에디터가 실행 중이지 않습니다.");
            return await app.Dispatcher.InvokeAsync(() => CreateScriptFileOnEditor(project, path, content)).Task.ConfigureAwait(false);
        }, path).ConfigureAwait(false);
    }

    private static string CreateScriptFileOnEditor(PixelToolProjectInfo project, string path, string content)
    {
        content ??= "";
        var files = new ProjectFileAccess(project.ProjectPath);
        string fullPath = files.Resolve(path);
        string relative = Path.GetRelativePath(files.Root, fullPath).Replace('\\', '/');
        if (!relative.StartsWith("Asset/", StringComparison.OrdinalIgnoreCase))
            return FileFailure(path, "outside_asset", ProjectFileAccess.AssetMutationDenied);
        string extension = Path.GetExtension(fullPath);
        if (!extension.Equals(".lua", StringComparison.OrdinalIgnoreCase) &&
            !extension.Equals(".pxm", StringComparison.OrdinalIgnoreCase))
            return FileFailure(relative, "unsupported_extension", ".lua 또는 .pxm 파일만 만들 수 있습니다.");
        if (File.Exists(fullPath) || Directory.Exists(fullPath))
            return FileFailure(relative, "already_exists", "같은 전체 파일명이 이미 있습니다.");
        string? placement = new ProjectAssetLayout(project.ProjectPath).GetCreationIssue(relative);
        if (placement != null) return FileFailure(relative, "invalid_location", placement);
        if (extension.Equals(".pxm", StringComparison.OrdinalIgnoreCase))
        {
            try { content = LuaFileManager.PrepareNewPxmContent(content); }
            catch (InvalidOperationException ex) { return FileFailure(relative, "invalid_script_template", ex.Message); }
        }
        byte[] bytes = new UTF8Encoding(false).GetBytes(content);
        if (bytes.Length > 256 * 1024) return FileFailure(relative, "content_too_large", "파일 내용은 256KB를 넘을 수 없습니다.");
        files.Resolve(relative); // 승인 중 경로가 바뀌었는지 다시 확인한다.
        Directory.CreateDirectory(Path.GetDirectoryName(fullPath)!);
        try
        {
            using var stream = new FileStream(fullPath, FileMode.CreateNew, FileAccess.Write, FileShare.None);
            stream.Write(bytes);
        }
        catch (IOException) when (File.Exists(fullPath) || Directory.Exists(fullPath))
        {
            return FileFailure(relative, "already_exists", "같은 전체 파일명이 이미 있습니다.");
        }
        GlobalFunction.GetDockedWindow<AssetWindow>()?.RefreshAfterAiChange();
        return ToolSuccess(new { path = relative,
            sha256 = Convert.ToHexString(SHA256.HashData(bytes)) });
    }

    [McpServerTool(Name = "Asset_File_Delete", ReadOnly = false, Destructive = true, OpenWorld = false),
     Description("Keywords: Asset. Move one existing Asset/ file to the Windows Recycle Bin. Input: project-relative path; optional expectedHash (whole-file SHA256) when known. Reject folders, changed files and unsaved Lua buffers. Success: {success:true,data:{path,recycleBin:true}}; failure: {success:false,path,error,message}. No permanent deletion.")]
    public static async Task<string> DeleteAssetFile(PixelToolProjectInfo project, string path, string expectedHash = "")
    {
        return await ToolResultAsync(async () =>
        {
            var app = Application.Current ?? throw new InvalidOperationException("PixelTool 에디터가 실행 중이지 않습니다.");
            return await app.Dispatcher.InvokeAsync(() => DeleteAssetFileOnEditor(project, path, expectedHash)).Task.ConfigureAwait(false);
        }, path).ConfigureAwait(false);
    }

    private static string DeleteAssetFileOnEditor(PixelToolProjectInfo project, string path, string expectedHash)
    {
        expectedHash ??= "";
        var files = new ProjectFileAccess(project.ProjectPath);
        string fullPath = files.Resolve(path);
        string relative = Path.GetRelativePath(files.Root, fullPath).Replace('\\', '/');
        if (!relative.StartsWith("Asset/", StringComparison.OrdinalIgnoreCase))
            return FileFailure(path, "outside_asset", ProjectFileAccess.AssetMutationDenied);
        if (!File.Exists(fullPath)) return FileFailure(relative, "not_found", "파일이 없거나 폴더입니다.");
        if (GlobalFunction.GetDockedWindow<LuaEditorWindow>()?.HasUnsavedFile(fullPath) == true)
            return FileFailure(relative, "unsaved_buffer", "에디터에 저장되지 않은 변경이 있습니다.");
        string initialHash = HashFile(fullPath);
        if (expectedHash.Length != 0 && !initialHash.Equals(expectedHash, StringComparison.OrdinalIgnoreCase))
            return FileFailure(relative, "stale_hash", "파일이 변경되었습니다. 다시 읽고 삭제하세요.");
        files.Resolve(relative);
        if (GlobalFunction.GetDockedWindow<LuaEditorWindow>()?.HasUnsavedFile(fullPath) == true)
            return FileFailure(relative, "unsaved_buffer", "에디터에 저장되지 않은 변경이 있습니다.");
        if (!HashFile(fullPath).Equals(initialHash, StringComparison.OrdinalIgnoreCase))
            return FileFailure(relative, "stale_hash", "삭제 전에 파일이 변경되었습니다.");
        Microsoft.VisualBasic.FileIO.FileSystem.DeleteFile(fullPath,
            Microsoft.VisualBasic.FileIO.UIOption.OnlyErrorDialogs,
            Microsoft.VisualBasic.FileIO.RecycleOption.SendToRecycleBin,
            Microsoft.VisualBasic.FileIO.UICancelOption.ThrowException);
        if (File.Exists(fullPath)) return FileFailure(relative, "delete_incomplete", "파일이 남아 있어 삭제되지 않았습니다.");
        GlobalFunction.GetDockedWindow<AssetWindow>()?.RefreshAfterAiChange();
        return ToolSuccess(new { path = relative, recycleBin = true });
    }

    private static string FileFailure(string path, string error, string message) =>
        JsonSerializer.Serialize(new { success = false, path, error, message });

    private static string ToolSuccess(object data) => JsonSerializer.Serialize(new { success = true, data });

    private static string ToolFailure(string error, string message) =>
        JsonSerializer.Serialize(new { success = false, error, message });

    private static string OperationName(string caller) => typeof(PixelToolProjectTools)
        .GetMethod(caller, BindingFlags.Public | BindingFlags.Static)?.GetCustomAttribute<McpServerToolAttribute>()?.Name ?? caller;

    private static string ToolResult(Func<string> action, string target = "", [CallerMemberName] string caller = "")
    {
        string operation = "MCP " + OperationName(caller);
        try
        {
            string result = WrapResult(action());
            LogResult(operation, target, result);
            return result;
        }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            AiActionLog.Failed(operation, target, ex);
            if (ex is ArgumentException or InvalidOperationException or IOException or UnauthorizedAccessException)
                return ToolFailure(ErrorCode(ex), ex.Message);
            throw;
        }
    }

    private static async Task<string> ToolResultAsync(Func<Task<string>> action, string target = "", [CallerMemberName] string caller = "")
    {
        string operation = "MCP " + OperationName(caller);
        try
        {
            string result = WrapResult(await action().ConfigureAwait(false));
            LogResult(operation, target, result);
            return result;
        }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            AiActionLog.Failed(operation, target, ex);
            if (ex is ArgumentException or InvalidOperationException or IOException or UnauthorizedAccessException)
                return ToolFailure(ErrorCode(ex), ex.Message);
            throw;
        }
    }

    private static void LogResult(string operation, string target, string result)
    {
        // These FileFailure codes are returned before target mutation in the functions above/below.
        // already_exists is intentionally excluded: a failed create/write may also return it.
        using var doc = JsonDocument.Parse(result);
        bool unchanged = doc.RootElement.TryGetProperty("success", out var success) && success.ValueKind == JsonValueKind.False &&
            doc.RootElement.TryGetProperty("path", out _) && doc.RootElement.TryGetProperty("error", out var error) &&
            error.GetString() is "outside_asset" or "unsupported_extension" or "invalid_location" or "invalid_script_template" or
                "content_too_large" or "not_found" or "unsaved_buffer" or "stale_hash" or "invalid_patch" or "ambiguous_text";
        AiActionLog.Record(operation, target, result, changeState: unchanged ? "대상변경없음" : "확인불가",
            stage: unchanged ? "사전검증" : "도구결과");
    }

    private static string WrapResult(string result)
    {
        try
        {
            using var json = JsonDocument.Parse(result);
            if (json.RootElement.ValueKind == JsonValueKind.Object && json.RootElement.TryGetProperty("success", out _)) return result;
            return ToolSuccess(json.RootElement);
        }
        catch (JsonException) { return ToolFailure("invalid_result", "도구 반환 형식이 올바르지 않습니다."); }
    }

    private static string ErrorCode(Exception ex) => ex switch
    {
        FileNotFoundException => "not_found",
        UnauthorizedAccessException => "access_denied",
        IOException => "io_error",
        ArgumentException => "invalid_input",
        _ => "invalid_operation"
    };

    private static string HashFile(string path)
    {
        using var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
        return Convert.ToHexString(SHA256.HashData(stream));
    }

    [McpServerTool(Name = "Asset_File_ReadLines", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Keywords: Asset. Read 1-100 saved text lines with whole-file SHA256. Input: path, startLine, lineCount, startColumn. Success: {success:true,data:{lines,sha256,nextStartLine,...}}; failure: {success:false,error,message}. Asset, Editor and Engine reads are allowed; Engine remains read-only.")]
    public static string ReadProjectFileLines(PixelToolProjectInfo project, string path, int startLine = 1, int lineCount = 50, int startColumn = 0)
        => ToolResult(() => ReadProjectFileLinesCore(project, path, startLine, lineCount, startColumn), path);

    private static string ReadProjectFileLinesCore(PixelToolProjectInfo project, string path, int startLine, int lineCount, int startColumn)
    {
        if (startLine < 1 || lineCount is < 1 or > 100 || startColumn < 0)
            throw new ArgumentOutOfRangeException(nameof(lineCount));
        var files = new ProjectFileAccess(project.ProjectPath);
        string fullPath = files.Resolve(path);
        if (!ProjectFileAccess.TextExtensions.Contains(Path.GetExtension(fullPath)))
            throw new InvalidOperationException("지원하는 텍스트 파일만 읽을 수 있습니다.");
        byte[] bytes = ProjectFileAccess.ReadBounded(fullPath);
        string text = new UTF8Encoding(false, true).GetString(bytes).TrimStart('\uFEFF');
        string[] lines = text.Replace("\r\n", "\n").Replace('\r', '\n').Split('\n');
        if (startLine > lines.Length) throw new ArgumentOutOfRangeException(nameof(startLine));
        if (startColumn > lines[startLine - 1].Length) throw new ArgumentOutOfRangeException(nameof(startColumn));
        if (startColumn > 0 && startColumn < lines[startLine - 1].Length &&
            char.IsLowSurrogate(lines[startLine - 1][startColumn]) && char.IsHighSurrogate(lines[startLine - 1][startColumn - 1]))
            throw new ArgumentException("문자 중간에서 시작할 수 없습니다.", nameof(startColumn));
        int endLine = Math.Min(lines.Length, startLine + lineCount - 1);
        int nextLine = startLine, nextColumn = startColumn, remaining = 12000;
        var page = new List<object>();
        while (nextLine <= endLine && remaining > 0)
        {
            string line = lines[nextLine - 1];
            int count = Math.Min(line.Length - nextColumn, remaining);
            if (count > 0 && nextColumn + count < line.Length && char.IsHighSurrogate(line[nextColumn + count - 1]) &&
                char.IsLowSurrogate(line[nextColumn + count])) count--;
            if (count == 0 && nextColumn < line.Length) break;
            page.Add(new { number = nextLine, column = nextColumn, text = line.Substring(nextColumn, count),
                complete = nextColumn + count == line.Length });
            remaining -= count + 32;
            nextColumn += count;
            if (nextColumn < line.Length) break;
            nextLine++;
            nextColumn = 0;
        }
        return JsonSerializer.Serialize(new { path = Path.GetRelativePath(files.Root, fullPath).Replace('\\', '/'),
            sha256 = Convert.ToHexString(SHA256.HashData(bytes)), totalLines = lines.Length,
            lines = page, nextStartLine = nextLine, nextStartColumn = nextColumn,
            truncated = nextLine <= endLine, hasMore = nextLine <= lines.Length });
    }

    [McpServerTool(Name = "Asset_File_Patch", ReadOnly = false, Destructive = false, OpenWorld = false),
     Description("Keywords: Asset. Replace one exact unique oldText span in an existing Asset text file; the prior Editor/AINavigation.md exception is preserved. Input: path, expectedHash (whole-file SHA256), oldText, newText. Success: {success:true,data:{path,sha256}}; failure: {success:false,path,error,message}. Reject stale hashes, missing/repeated text and unsaved Lua buffers; preserve other bytes.")]
    public static async Task<string> PatchProjectFile(PixelToolProjectInfo project, string path, string expectedHash,
        string oldText, string newText)
    {
        return await ToolResultAsync(async () =>
        {
            var app = Application.Current ?? throw new InvalidOperationException("PixelTool 에디터가 실행 중이지 않습니다.");
            return await app.Dispatcher.InvokeAsync(() => PatchProjectFileOnEditor(project, path, expectedHash, oldText, newText)).Task.ConfigureAwait(false);
        }, path).ConfigureAwait(false);
    }

    private static string PatchProjectFileOnEditor(PixelToolProjectInfo project, string path, string expectedHash,
        string oldText, string newText)
    {
        expectedHash ??= "";
        newText ??= "";
        var files = new ProjectFileAccess(project.ProjectPath);
        string fullPath = files.Resolve(path);
        string relative = Path.GetRelativePath(files.Root, fullPath).Replace('\\', '/');
        if (!relative.StartsWith("Asset/", StringComparison.OrdinalIgnoreCase) &&
            !relative.Equals(ProjectNavigation.NotesPath, StringComparison.OrdinalIgnoreCase))
            return FileFailure(path, "outside_asset", ProjectFileAccess.AssetMutationDenied);
        if (!ProjectFileAccess.TextExtensions.Contains(Path.GetExtension(fullPath)))
            return FileFailure(relative, "unsupported_extension", "지원하는 텍스트 파일만 수정할 수 있습니다.");
        if (string.IsNullOrEmpty(oldText) || oldText == newText)
            return FileFailure(relative, "invalid_patch", "서로 다른 원문과 수정문이 필요합니다.");
        if (GlobalFunction.GetDockedWindow<LuaEditorWindow>()?.HasUnsavedFile(fullPath) == true)
            return FileFailure(relative, "unsaved_buffer", "에디터에 저장되지 않은 변경이 있습니다. 먼저 저장하거나 닫아 주세요.");
        if (!File.Exists(fullPath)) return FileFailure(relative, "not_found", "수정할 파일이 없습니다.");
        byte[] original = ProjectFileAccess.ReadBounded(fullPath);
        string actualHash = Convert.ToHexString(SHA256.HashData(original));
        if (!actualHash.Equals(expectedHash, StringComparison.OrdinalIgnoreCase))
            return FileFailure(relative, "stale_hash", "파일이 변경되었습니다. 다시 읽고 수정하세요.");
        string before = new UTF8Encoding(false, true).GetString(original);
        int offset = before.IndexOf(oldText, StringComparison.Ordinal);
        if (offset < 0 || before.IndexOf(oldText, offset + 1, StringComparison.Ordinal) >= 0)
            return FileFailure(relative, "ambiguous_text", "기존 문구가 없거나 두 번 이상 나옵니다.");
        string after = before[..offset] + newText + before[(offset + oldText.Length)..];
        byte[] replacement = new UTF8Encoding(false, true).GetBytes(after);
        if (replacement.Length > 256 * 1024) return FileFailure(relative, "content_too_large", "수정 결과는 256KB까지입니다.");
        ProjectNavigation.CheckNotesSize(relative, replacement.Length);
        files.Resolve(relative);
        if (GlobalFunction.GetDockedWindow<LuaEditorWindow>()?.HasUnsavedFile(fullPath) == true ||
            !Convert.ToHexString(SHA256.HashData(ProjectFileAccess.ReadBounded(fullPath))).Equals(expectedHash, StringComparison.OrdinalIgnoreCase))
            return FileFailure(relative, "stale_hash", "승인 중 파일이나 편집 내용이 변경되었습니다. 다시 읽고 수정하세요.");
        string temporary = fullPath + "." + Guid.NewGuid().ToString("N") + ".tmp";
        try
        {
            File.WriteAllBytes(temporary, replacement);
            File.Move(temporary, fullPath, overwrite: true);
        }
        finally { if (File.Exists(temporary)) File.Delete(temporary); }
        return ToolSuccess(new { path = relative,
            sha256 = Convert.ToHexString(SHA256.HashData(replacement)) });
    }

    [McpServerTool(Name = "Editor_Log_Read", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Keywords: Editor. Read captured logs. Input: afterCursor, limit, level=errors|warnings|all. Success: {success:true,data:{entries,cursor,hasMore,...}}; failure: {success:false,error,message}. Continue while hasMore; cleared or truncated history cannot prove no errors.")]
    public static string ReadEditorLogs(long afterCursor = 0, int limit = 50, string level = "all")
        => ToolResult(() => ConsoleWindow.ReadLogsForAi(afterCursor, limit, level).GetRawText());

    [McpServerTool(Name = "Editor_Log_Clear", ReadOnly = false, Destructive = true, OpenWorld = false),
     Description("Keywords: Editor. Clear the displayed and captured editor log history only when explicitly requested. No input. Success: {success:true,data:{cleared,afterCursor}}; failure: {success:false,error,message}. Cleared history cannot be checked afterward.")]
    public static async Task<string> ClearEditorLogs()
    {
        return await ToolResultAsync(async () =>
        {
            var app = Application.Current ?? throw new InvalidOperationException("PixelTool 에디터가 실행 중이지 않습니다.");
            long cursor = await app.Dispatcher.InvokeAsync(() =>
                app.MainWindow is MainWindow window
                    ? window.ClearEditorLogsForAi()
                    : throw new InvalidOperationException("PixelTool 메인 창을 찾을 수 없습니다.")).Task.ConfigureAwait(false);
            return JsonSerializer.Serialize(new { cleared = true, afterCursor = cursor });
        }).ConfigureAwait(false);
    }

    [McpServerTool(Name = "Editor_Game_KeyInput", ReadOnly = false, Destructive = false, OpenWorld = false),
     Description("Keywords: Editor. In play mode, send one key for 1-120 frames. Input: key=A-Z|Space|Enter|Left|Up|Right|Down, frames. Success: {success:true,data:{key,frames,events,logs,...}}; failure: {success:false,error,message}. Use Editor_Log_Read for details.")]
    public static async Task<string> TestGameKey(string key, int frames = 1)
    {
        return await ToolResultAsync(async () =>
        {
            var app = Application.Current ?? throw new InvalidOperationException("PixelTool 에디터가 실행 중이지 않습니다.");
            var result = await app.Dispatcher.InvokeAsync(() =>
                app.MainWindow is MainWindow window
                    ? window.TestGameKeyForAiAsync(key, frames, CancellationToken.None)
                    : throw new InvalidOperationException("PixelTool 메인 창을 찾을 수 없습니다.")).Task.Unwrap().ConfigureAwait(false);
            return result.GetRawText();
        }, key).ConfigureAwait(false);
    }

    [McpServerTool(Name = "Editor_Game_PlayMode", ReadOnly = false, Destructive = false, OpenWorld = false),
     Description("Keywords: Editor. Set the toolbar to play mode. No input. Success: {success:true,data:{mode,changed}}; failure: {success:false,error,message}.")]
    public static Task<string> SetPlayMode() => SetEditorMode("play");

    [McpServerTool(Name = "Editor_Game_EditorMode", ReadOnly = false, Destructive = false, OpenWorld = false),
     Description("Keywords: Editor. Set the toolbar to editor mode. No input. Success: {success:true,data:{mode,changed}}; failure: {success:false,error,message}.")]
    public static Task<string> SetEditMode() => SetEditorMode("editor");

    private static async Task<string> SetEditorMode(string mode)
    {
        return await ToolResultAsync(async () =>
        {
            var app = Application.Current ?? throw new InvalidOperationException("PixelTool 에디터가 실행 중이지 않습니다.");
            var result = await app.Dispatcher.InvokeAsync(() =>
                app.MainWindow is MainWindow window
                    ? window.SetEditorModeForAi(mode)
                    : throw new InvalidOperationException("PixelTool 메인 창을 찾을 수 없습니다.")).Task.ConfigureAwait(false);
            return result.GetRawText();
        }, mode, caller: mode == "play" ? nameof(SetPlayMode) : nameof(SetEditMode)).ConfigureAwait(false);
    }

    [McpServerTool(Name = "Editor_Lua_Highlight", ReadOnly = false, Destructive = false, OpenWorld = false),
     Description("Keywords: Editor. Open a project Lua/PXM file and highlight 1-based lines. Input: path, startLine, endLine. Success: {success:true,data:{result}}; failure: {success:false,error,message}. Uses current editor buffer; does not change file content.")]
    public static async Task<string> HighlightLuaLines(string path, int startLine, int endLine)
    {
        return await ToolResultAsync(async () =>
        {
            var app = Application.Current ?? throw new InvalidOperationException("PixelTool 에디터가 실행 중이지 않습니다.");
            string result = await app.Dispatcher.InvokeAsync(() =>
                app.MainWindow is MainWindow window
                    ? window.HighlightLuaLines(path, startLine, endLine)
                    : "PixelTool 메인 창을 찾을 수 없습니다.").Task.ConfigureAwait(false);
            return result.StartsWith("표시됨: ", StringComparison.Ordinal)
                ? ToolSuccess(new { result })
                : ToolFailure("highlight_failed", result);
        }, path).ConfigureAwait(false);
    }
}
