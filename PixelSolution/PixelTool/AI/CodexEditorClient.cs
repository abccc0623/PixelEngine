using System.Text.Json;
using System.Text.Json.Nodes;

namespace PixelTool;

// Keeps editor-only operations on the existing permission-checked tool path.
internal sealed class CodexEditorClient : IAsyncDisposable
{
    private readonly CodexChatClient _server = new();
    private readonly CancellationTokenSource _stop = new();
    private CancellationTokenSource? _turnStop;
    private ChatGptProjectTools? _tools;
    private string _threadId = "";
    private bool _allowEdits, _confirmActions = true;
    public event Action<string, JsonElement>? Notification;
    public event Action<string>? Disconnected;
    public Func<string, JsonElement, Task<bool>>? Approve { get; set; }
    public Func<Task<ChatGptProjectTools.ImportedAsset?>>? PickAsset { get; set; }
    public Func<CancellationToken, Task<JsonElement>>? ReimportProject { get; set; }
    public Func<long, int, JsonElement>? ReadEditorLogs { get; set; }
    public Func<string, int, int, Task<string>>? HighlightLuaLines { get; set; }
    public Func<string, Task<bool>>? CanDeleteAssetFile { get; set; }
    public Func<string, string[], CancellationToken, Task<string>>? AskChoice { get; set; }

    internal CodexEditorClient()
    {
        _server.Notification += (method, p) =>
        {
            if (method == "turn/completed") _turnStop?.Cancel();
            Notification?.Invoke(method, p);
        };
        _server.Disconnected += details =>
        {
            try { _turnStop?.Cancel(); } catch (ObjectDisposedException) { }
            Disconnected?.Invoke(details);
        };
        _server.CallTool = InvokeToolAsync;
        // No native command/file escalation may bypass the editor's Asset-only tools.
        _server.Approve = (_, _) => Task.FromResult(false);
    }

    public Task ConnectAsync(string project)
    {
        _tools = new ChatGptProjectTools(project);
        return _server.ConnectAsync(project);
    }

    public async Task<JsonElement> RequestAsync(string method, object parameters)
    {
        var p = JsonSerializer.SerializeToNode(parameters)!.AsObject();
        if (method == "account/list")
        {
            var sessions = await _server.RequestAsync("accountSession/list", new { refreshWorkspaceMetadata = false }).ConfigureAwait(false);
            var data = sessions.GetProperty("sessions").EnumerateArray().Select(session => new
            {
                id = session.GetProperty("sessionId").GetString(),
                label = session.TryGetProperty("email", out var email) ? email.GetString() ?? "ChatGPT" : "ChatGPT"
            }).ToArray();
            return JsonSerializer.SerializeToElement(new { data, selected = sessions.GetProperty("activeSessionId").GetString() });
        }
        if (method == "account/select")
        {
            var sessions = await _server.RequestAsync("accountSession/list", new { refreshWorkspaceMetadata = false }).ConfigureAwait(false);
            var session = sessions.GetProperty("sessions").EnumerateArray().First(s => s.GetProperty("sessionId").GetString() == p["id"]!.GetValue<string>());
            string? accountId = session.GetProperty("selectedWorkspaceAccountId").GetString();
            accountId ??= session.GetProperty("workspaces").EnumerateArray().First().GetProperty("accountId").GetString();
            return await _server.RequestAsync("accountSession/switch", new { sessionId = p["id"]!.GetValue<string>(), accountId, allowBrowserLogin = false }).ConfigureAwait(false);
        }
        if (method == "account/login/start") p.Remove("newAccount");
        if (method is "thread/start" or "thread/resume")
        {
            p["sandbox"] = "read-only";
            p["approvalPolicy"] = "never";
            p["developerInstructions"] = (p["developerInstructions"]?.GetValue<string>() ?? "") +
                "\nYou are AI 자비스 in PixelTool. Use only pixeltool tools for project changes, questions, editor reimport and logs. " +
                "Native shell/file operations are unavailable. After script changes reimport and fix reported errors. " +
                "Use focused line reads; preserve existing files and use exact patches. End with a short result/change summary; code changes are shown collapsed by the editor.";
            if (method == "thread/start")
            {
                var tools = ChatGptProjectTools.Definitions.Select(definition =>
                {
                    var tool = JsonSerializer.SerializeToElement(definition);
                    return new { type = "function", name = tool.GetProperty("name").GetString(),
                        description = tool.GetProperty("description").GetString(), inputSchema = tool.GetProperty("parameters") };
                }).ToArray();
                p["dynamicTools"] = JsonSerializer.SerializeToNode(new[] { new { type = "namespace", name = "pixeltool",
                    description = "PixelTool project and editor operations", tools } });
            }
            var result = await _server.RequestAsync(method, p).ConfigureAwait(false);
            _threadId = result.GetProperty("thread").GetProperty("id").GetString()!;
            return result;
        }
        if (method == "turn/start")
        {
            _allowEdits = p["sandboxPolicy"]?["type"]?.GetValue<string>() == "workspaceWrite";
            _confirmActions = p["confirmActions"]?.GetValue<bool>() ?? true;
            p.Remove("confirmActions");
            // The CLI cannot write around our tool permissions, even in auto-approve mode.
            p["sandboxPolicy"] = JsonSerializer.SerializeToNode(new { type = "readOnly" });
            p["approvalPolicy"] = "never";
            _turnStop?.Dispose();
            _turnStop = CancellationTokenSource.CreateLinkedTokenSource(_stop.Token);
        }
        if (method == "turn/interrupt") _turnStop?.Cancel();
        return await _server.RequestAsync(method, p).ConfigureAwait(false);
    }

    private void Emit(string method, object parameters) => Notification?.Invoke(method, JsonSerializer.SerializeToElement(parameters));

    private async Task<object> InvokeToolAsync(JsonElement p)
    {
        string result;
        bool success = true;
        string name = p.GetProperty("tool").GetString()!;
        string turnId = p.GetProperty("turnId").GetString()!;
        var token = _turnStop?.Token ?? _stop.Token;
        try
        {
            token.ThrowIfCancellationRequested();
            if (p.GetProperty("threadId").GetString() != _threadId ||
                !p.TryGetProperty("namespace", out var ns) || ns.GetString() != "pixeltool")
                throw new InvalidOperationException("현재 대화의 에디터 도구만 호출할 수 있습니다.");
            var args = p.GetProperty("arguments");
            async Task<bool> ConfirmOperation(string path, string before, string after)
            {
                if (Approve == null) return false;
                var request = JsonSerializer.SerializeToElement(new { threadId = _threadId, turnId,
                    itemId = p.GetProperty("callId").GetString(), path, operation = name,
                    previousContent = before, proposedContent = after });
                return await Approve(name == "reimport_project" ? "item/commandExecution/requestApproval" : "item/fileChange/requestApproval", request)
                    .WaitAsync(token).ConfigureAwait(false);
            }
            if (name == "ask_choice")
            {
                string question = args.GetProperty("question").GetString() ?? "";
                string[] options = args.GetProperty("options").EnumerateArray().Select(o => o.GetString() ?? "").ToArray();
                if (AskChoice == null || string.IsNullOrWhiteSpace(question) || question.Length > 500 || options.Length is < 2 or > 5 ||
                    options.Any(o => string.IsNullOrWhiteSpace(o) || o.Length > 150) || options.Distinct().Count() != options.Length)
                    throw new InvalidOperationException("질문과 서로 다른 선택지 2~5개가 필요합니다.");
                result = JsonSerializer.Serialize(new { question, answer = await AskChoice(question, options, token).WaitAsync(token).ConfigureAwait(false) });
            }
            else if (name == "read_editor_logs")
                result = (ReadEditorLogs ?? throw new InvalidOperationException("로그 도구가 연결되지 않았습니다."))(
                    args.GetProperty("afterCursor").GetInt64(), args.GetProperty("limit").GetInt32()).GetRawText();
            else if (name == "highlight_lua_lines")
                result = JsonSerializer.Serialize(new { result = await (HighlightLuaLines ?? throw new InvalidOperationException("에디터 표시 도구가 없습니다."))(
                    args.GetProperty("path").GetString()!, args.GetProperty("startLine").GetInt32(), args.GetProperty("endLine").GetInt32()).WaitAsync(token).ConfigureAwait(false) });
            else if (name == "reimport_project")
            {
                if (!_allowEdits) throw new InvalidOperationException("프로젝트 파일 수정 허용이 꺼져 있습니다.");
                if (_confirmActions && !await ConfirmOperation("Asset/main.lua", "", "리임포트: 현재 런타임을 초기화합니다.").ConfigureAwait(false))
                    throw new InvalidOperationException("리임포트를 승인하지 않았습니다.");
                result = (await (ReimportProject ?? throw new InvalidOperationException("리임포트 도구가 없습니다."))(token).ConfigureAwait(false)).GetRawText();
            }
            else
                result = await _tools!.InvokeAsync(name, args, _allowEdits, _confirmActions, ConfirmOperation, token, PickAsset,
                    (path, before, after) => Emit("item/completed", new { threadId = _threadId, turnId,
                        item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", path, before, after } }), CanDeleteAssetFile).ConfigureAwait(false);
            if (name is "write_project_file" or "patch_project_file" or "delete_asset_file" or "delete_asset_folder" or "move_asset" or "create_asset_folder" or "create_png_asset" or "copy_asset" or "import_asset")
                Emit("item/completed", new { threadId = _threadId, turnId, item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", result } });
        }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            success = false;
            result = "도구 작업 실패: " + ex.Message;
            if (!token.IsCancellationRequested) Emit("turn/toolWarning", new { threadId = _threadId, turnId, message = result });
        }
        return new { success, contentItems = new[] { new { type = "inputText", text = result } } };
    }

    public async ValueTask DisposeAsync()
    {
        _stop.Cancel();
        _turnStop?.Cancel();
        await _server.DisposeAsync().ConfigureAwait(false);
    }
}
