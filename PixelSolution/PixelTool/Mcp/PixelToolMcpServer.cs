using System.ComponentModel;
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
    [McpServerTool(Name = "get_project_info", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Returns the project and Asset/Engine/Editor paths opened in this PixelTool instance. Read-only; does not access or modify engine objects.")]
    public static string GetProjectInfo(PixelToolProjectInfo project)
    {
        return JsonSerializer.Serialize(project);
    }

    [McpServerTool(Name = "get_editor_context", ReadOnly = true, Destructive = false, OpenWorld = false),
     Description("Returns the current Lua editor file, caret, selected line range and selected text. For code changes, inspect this file/selection first and expand to directly related declarations or callers only when needed. Read-only snapshot; empty FilePath means no file is open.")]
    public static string GetEditorContext()
    {
        return JsonSerializer.Serialize(PixelToolEditorContextSnapshot.Current);
    }

    [McpServerTool(Name = "highlight_lua_lines", ReadOnly = false, Destructive = false, OpenWorld = false),
     Description("Open a project Lua file in the PixelTool Lua editor and highlight a 1-based line range. Use after locating the requested code. path is relative to the open project, e.g. Asset/Script/main.lua. Selects the current in-memory document, including unsaved edits. Does not change file content.")]
    public static async Task<string> HighlightLuaLines(string path, int startLine, int endLine)
    {
        var app = Application.Current;
        if (app == null) return "PixelTool 에디터가 실행 중이지 않습니다.";
        return await app.Dispatcher.InvokeAsync(() =>
            app.MainWindow is MainWindow window
                ? window.HighlightLuaLines(path, startLine, endLine)
                : "PixelTool 메인 창을 찾을 수 없습니다.").Task.ConfigureAwait(false);
    }
}
