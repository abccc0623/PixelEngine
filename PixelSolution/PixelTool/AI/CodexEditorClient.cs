using System.IO;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace PixelTool;

// Keeps editor-only operations on the existing permission-checked tool path.
internal sealed class CodexEditorClient : IAsyncDisposable
{
    private static readonly HashSet<string> SupportedTools = ChatGptProjectTools.Definitions
        .Select(definition => JsonSerializer.SerializeToElement(definition).GetProperty("name").GetString()!)
        .ToHashSet(StringComparer.Ordinal);
    // UI-only correlation metadata; never serialized into CLI requests or tool replies.
    internal string? LocalLogRequestId { get; set; }
    private string? _startingLogRequestId;
    private readonly System.Collections.Concurrent.ConcurrentDictionary<string, string> _logRequests = new();
    private readonly CodexChatClient _server = new();
    private readonly CancellationTokenSource _stop = new();
    private CancellationTokenSource? _turnStop;
    private ChatGptProjectTools? _tools;
    private string _threadId = "";
    private string _project = "";
    private string? _activeTurnId;
    private bool _newThread;
    private bool _startingTurn;
    private string? _interruptedTurnId;
    private readonly HashSet<string> _finishedTurns = new(StringComparer.Ordinal);
    private readonly SemaphoreSlim _toolGate = new(1, 1);
    private readonly CodexUsageTracker _usage = new();
    private bool _allowEdits = true;
    public event Action<string, JsonElement>? Notification;
    public event Action<string>? Disconnected;
    public Func<string, JsonElement, Task<bool>>? Approve { get; set; }
    public Func<Task<ChatGptProjectTools.ImportedAsset?>>? PickAsset { get; set; }
    public Func<CancellationToken, Task<JsonElement>>? ReimportProject { get; set; }
    public Func<long, int, JsonElement>? ReadEditorLogs { get; set; }
    public Func<Task<long>>? ClearEditorLogs { get; set; }
    public Func<string, int, int, CancellationToken, Task<JsonElement>>? TestGameKey { get; set; }
    public Func<string, Task<JsonElement>>? SetEditorMode { get; set; }
    public Func<string, int, int, Task<string>>? HighlightLuaLines { get; set; }
    public Func<string, Task<bool>>? CanDeleteAssetFile { get; set; }
    public Func<string, string[], CancellationToken, Task<string>>? AskChoice { get; set; }

    internal CodexEditorClient()
    {
        _server.Notification += (method, p) =>
        {
            bool ownThread = p.ValueKind == JsonValueKind.Object && p.TryGetProperty("threadId", out var thread) && thread.GetString() == _threadId;
            if (ownThread && method == "turn/started")
            {
                string id = p.GetProperty("turn").GetProperty("id").GetString()!;
                lock (_finishedTurns)
                {
                    if (!_startingTurn || _finishedTurns.Contains(id) || (_activeTurnId != null && _activeTurnId != id)) return;
                    _activeTurnId = id;
                    if (_startingLogRequestId != null) _logRequests[id] = _startingLogRequestId;
                }
                // Stop can be clicked before turn/start returns its ID. Local tools are
                // already canceled; interrupt the CLI as soon as that ID becomes known.
                if (_turnStop?.IsCancellationRequested == true) _ = InterruptStartedTurnAsync(id);
            }
            if (ownThread && method == "thread/tokenUsage/updated") ObserveUsage(p);
            if (ownThread && method == "turn/completed")
            {
                string id = p.GetProperty("turn").GetProperty("id").GetString()!;
                lock (_finishedTurns)
                {
                    if (id != _activeTurnId || !_finishedTurns.Add(id)) return;
                    _startingTurn = false;
                    _turnStop?.Cancel();
                }
                _ = CompleteTurnAsync(p.Clone());
                return;
            }
            Emit(method, p);
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
        _project = project; _tools = new ChatGptProjectTools(project);
        return _server.ConnectAsync(project);
    }

    public async Task<JsonElement> RequestAsync(string method, object parameters, CancellationToken requestToken = default)
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
                "Native shell/file operations are unavailable. Their read-only sandbox does not describe PixelTool editor permissions. " +
                "Use pixeltool_assets MCP Asset_File_Search to locate an exact file path first. Asset_FileContent_Search requires that exact file path; never pass a folder or guessed path. Use pixeltool tools for remaining Asset reads, writes, moves and renames. " +
                "If permission is uncertain, call pixeltool.get_asset_permissions; the CLI sandbox does not govern PixelTool project tools. " +
                "Run reimport and error checks when the user requests validation. " +
                "Use focused line reads; preserve existing files and use exact patches. Keep replies brief. For implementation choices, use a supported default from current project and engine docs and continue. Do not interrupt work with a choice question. If essential user-only information still blocks progress, report the exact blocker in the final answer.";
            p["developerInstructions"] = p["developerInstructions"]!.GetValue<string>() + CodexInstructions.Text;
            if (method == "thread/start")
            {
                var tools = ChatGptProjectTools.Definitions
                    .Select(definition =>
                    {
                        var d = JsonSerializer.SerializeToElement(definition);
                        return new { type = "function", name = d.GetProperty("name").GetString(),
                            description = d.GetProperty("description").GetString(), inputSchema = d.GetProperty("parameters") };
                    }).ToArray();
                p["dynamicTools"] = JsonSerializer.SerializeToNode(new[] { new { type = "namespace", name = "pixeltool",
                    description = "PixelTool project and editor operations", tools } });
            }
            var result = await _server.RequestAsync(method, p).ConfigureAwait(false);
            _threadId = result.GetProperty("thread").GetProperty("id").GetString()!;
            _newThread = method == "thread/start";
            return result;
        }
        if (method == "turn/start")
        {
            _tools?.BeginTurn();
            _startingLogRequestId = LocalLogRequestId;
            using var requestLogScope = AiActionLog.Enter(_startingLogRequestId);
            _turnStop?.Cancel();
            requestToken.ThrowIfCancellationRequested();
            lock (_finishedTurns)
            {
                _activeTurnId = _interruptedTurnId = null;
                _turnStop?.Dispose();
                _turnStop = CancellationTokenSource.CreateLinkedTokenSource(_stop.Token, requestToken);
                _startingTurn = true;
            }
            p.Remove("confirmActions");
            p.Remove("requirePlan");
            _usage.BeginWork(_threadId, _newThread);
            _newThread = false;
            EmitUsage();
            // The CLI cannot write around our tool permissions, even in auto-approve mode.
            p["sandboxPolicy"] = JsonSerializer.SerializeToNode(new { type = "readOnly" });
            p["approvalPolicy"] = "never";
            requestToken.ThrowIfCancellationRequested();
            var started = await _server.RequestAsync(method, p).ConfigureAwait(false);
            string id = started.GetProperty("turn").GetProperty("id").GetString()!;
            lock (_finishedTurns)
            {
                if (_finishedTurns.Contains(id)) return started;
                if (_activeTurnId != null && _activeTurnId != id)
                    throw new InvalidOperationException("CLI returned a different active turn.");
                _activeTurnId = id;
                if (_startingLogRequestId != null) _logRequests[id] = _startingLogRequestId;
            }
            if (_turnStop.IsCancellationRequested) await InterruptStartedTurnAsync(id).ConfigureAwait(false);
            return started;
        }
        if (method == "turn/interrupt")
        {
            if (p["threadId"]?.GetValue<string>() != _threadId) return JsonSerializer.SerializeToElement(new { });
            string? requestedTurn = p["turnId"]?.GetValue<string>();
            if (requestedTurn != null && requestedTurn != _activeTurnId) return JsonSerializer.SerializeToElement(new { });
            _turnStop?.Cancel();
            if (_activeTurnId != null) await InterruptStartedTurnAsync(_activeTurnId).ConfigureAwait(false);
            return JsonSerializer.SerializeToElement(new { });
        }
        return await _server.RequestAsync(method, p).ConfigureAwait(false);
    }

    private async Task CompleteTurnAsync(JsonElement completed)
    {
        // A write may have committed immediately before cancellation. Its receipt and
        // editor path refresh must be delivered before the UI enables a new request.
        try
        {
            await _toolGate.WaitAsync().ConfigureAwait(false);
            _toolGate.Release();
            Emit("turn/completed", completed);
        }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            _stop.Cancel();
            string details = "요청 종료 정리를 확인하지 못했습니다. " + CodexChatClient.SanitizeDiagnostic(ex.Message);
            try { await _server.DisposeAsync().ConfigureAwait(false); }
            catch (Exception cleanup) when (cleanup is not OutOfMemoryException) { details += "\n" + CodexChatClient.SanitizeDiagnostic(cleanup.Message); }
            Disconnected?.Invoke(details);
        }
    }
    private async Task InterruptStartedTurnAsync(string turnId)
    {
        lock (_finishedTurns)
        {
            if (_activeTurnId != turnId || _finishedTurns.Contains(turnId) || _interruptedTurnId == turnId) return;
            _interruptedTurnId = turnId;
        }
        try { await _server.RequestAsync("turn/interrupt", new { threadId = _threadId, turnId }).ConfigureAwait(false); }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            if (_activeTurnId == turnId)
                Emit("turn/toolWarning", new { threadId = _threadId, turnId,
                    message = "로컬 도구 작업은 중지했지만 AI 응답 중지 확인에 실패했습니다. 연결 상태를 확인하세요. " + ex.Message });
        }
    }

    private string? RequestFor(JsonElement p)
    {
        if (p.ValueKind != JsonValueKind.Object) return null;
        string? turn = p.TryGetProperty("turnId", out var id) && id.ValueKind == JsonValueKind.String ? id.GetString() : null;
        if (turn == null && p.TryGetProperty("turn", out var nested) && nested.ValueKind == JsonValueKind.Object && nested.TryGetProperty("id", out id))
            turn = id.GetString();
        return turn != null && _logRequests.TryGetValue(turn, out var request) ? request : null;
    }

    private void Emit(string method, object parameters)
    {
        var json = JsonSerializer.SerializeToElement(parameters);
        var local = JsonNode.Parse(json.GetRawText())!.AsObject();
        local["localLogRequestId"] = AiActionLog.CurrentRequestId ?? RequestFor(json);
        Notification?.Invoke(method, JsonSerializer.SerializeToElement(local));
    }

    private Task<object> InvokeToolAsync(JsonElement p) => InvokeToolCoreAsync(p);

    private async Task<object> InvokeToolCoreAsync(JsonElement p)
    {
        using var logScope = AiActionLog.Enter(RequestFor(p));
        string name = p.TryGetProperty("tool", out var tool) ? tool.GetString() ?? "unknown" : "unknown";
        string target = p.TryGetProperty("arguments", out var args) ? AiActionLog.Target(name, args) : "";
        try
        {
            var reply = await InvokeToolOperationAsync(p).ConfigureAwait(false);
            var json = JsonSerializer.SerializeToElement(reply);
            bool success = json.TryGetProperty("success", out var ok) && ok.ValueKind == JsonValueKind.True;
            string result = json.GetRawText();
            if (success && json.TryGetProperty("contentItems", out var items) && items.ValueKind == JsonValueKind.Array)
                foreach (var item in items.EnumerateArray())
                    if (item.TryGetProperty("text", out var text) && text.ValueKind == JsonValueKind.String)
                    { result = text.GetString() ?? ""; break; }
            bool loggedSuccess = AiActionLog.Record("AI 도구 " + name, target, result, success,
                stage: AiActionLog.FailureStage(result));
            Emit("turn/toolOutcome", new { threadId = _threadId, turnId = _activeTurnId, tool = name, success = loggedSuccess });
            return reply;
        }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            AiActionLog.Failed("AI 도구 " + name, target, ex);
            throw;
        }
    }

    private async Task<object> InvokeToolOperationAsync(JsonElement p)
    {
        string result;
        bool success = true;
        bool held = false;
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
            if (_activeTurnId != turnId) throw new InvalidOperationException("현재 요청의 도구만 호출할 수 있습니다.");
            if (!SupportedTools.Contains(name))
                throw new InvalidOperationException("등록되지 않은 도구입니다.");
            await _toolGate.WaitAsync(token).ConfigureAwait(false);
            held = true;
            token.ThrowIfCancellationRequested();
            if (_activeTurnId != turnId) throw new InvalidOperationException("현재 요청이 종료되었거나 바뀌었습니다.");
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
                    args.GetProperty("afterCursor").GetInt64(), Math.Clamp(args.GetProperty("limit").GetInt32(), 1, 100)).GetRawText();
            else if (name == "clear_editor_logs")
            {
                if (ClearEditorLogs == null) throw new InvalidOperationException("로그 삭제 도구가 연결되지 않았습니다.");
                long cursor = await ClearEditorLogs().WaitAsync(token).ConfigureAwait(false);
                result = JsonSerializer.Serialize(new { cleared = true, afterCursor = cursor });
            }
            else if (name == "test_game_key")
                result = (await (TestGameKey ?? throw new InvalidOperationException("게임 입력 도구가 연결되지 않았습니다."))(
                    args.TryGetProperty("keys", out var keys) && keys.ValueKind == JsonValueKind.Array
                        ? string.Join("+", keys.EnumerateArray().Select(item => item.GetString()))
                        : args.GetProperty("key").GetString()!,
                    args.GetProperty("frames").GetInt32(),
                    args.TryGetProperty("durationMs", out var duration) ? duration.GetInt32() : 0,
                    token).WaitAsync(token).ConfigureAwait(false)).GetRawText();
            else if (name == "set_editor_mode")
                result = (await (SetEditorMode ?? throw new InvalidOperationException("모드 전환 도구가 연결되지 않았습니다."))(
                    args.GetProperty("mode").GetString()!).WaitAsync(token).ConfigureAwait(false)).GetRawText();
            else if (name == "highlight_lua_lines")
                result = JsonSerializer.Serialize(new { result = await (HighlightLuaLines ?? throw new InvalidOperationException("에디터 표시 도구가 없습니다."))(
                    args.GetProperty("path").GetString()!, args.GetProperty("startLine").GetInt32(), args.GetProperty("endLine").GetInt32()).WaitAsync(token).ConfigureAwait(false) });
            else if (name == "reimport_project")
            {
                token.ThrowIfCancellationRequested();
                var imported = await (ReimportProject ?? throw new InvalidOperationException("리임포트 도구가 없습니다."))(token).ConfigureAwait(false);
                result = imported.GetRawText();
            }
            else
                result = await _tools!.InvokeAsync(name, args, _allowEdits, false, (_, _, _) => Task.FromResult(false), token, PickAsset,
                    (path, before, after) => Emit("item/completed", new { threadId = _threadId, turnId,
                        item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", path, before, after } }), CanDeleteAssetFile).ConfigureAwait(false);
            if (name is "write_project_file" or "patch_project_file" or "delete_asset_file" or "delete_asset_folder" or "move_asset" or "create_asset_folder" or "create_png_asset" or "copy_asset" or "import_asset")
                Emit("item/completed", new { threadId = _threadId, turnId, item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", result } });
        }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            success = false;
            result = ex.Message == ProjectFileAccess.AssetMutationDenied ? ex.Message : "도구 작업 실패: " + ex.Message;
            if (!token.IsCancellationRequested) Emit("turn/toolWarning", new { threadId = _threadId, turnId, message = result });
        }
        finally { if (held) _toolGate.Release(); }
        return new { success, contentItems = new[] { new { type = "inputText", text = result } } };
    }

    private void ObserveUsage(JsonElement usage)
    {
        _usage.Observe(usage);
        EmitUsage();
    }

    private void EmitUsage() => Emit("turn/usage", new { threadId = _threadId, text = _usage.Snapshot().Text });

    public async ValueTask DisposeAsync()
    {
        _stop.Cancel();
        _turnStop?.Cancel();
        await _server.DisposeAsync().ConfigureAwait(false);
    }
}
