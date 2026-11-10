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
    private string _project = "", _model = "";
    private string? _activeTurnId;
    private bool _newThread;
    private CodexWorkSession? _work;
    private readonly SemaphoreSlim _toolGate = new(1, 1);
    private readonly CodexUsageTracker _usage = new();
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
            bool ownThread = p.ValueKind == JsonValueKind.Object && p.TryGetProperty("threadId", out var thread) && thread.GetString() == _threadId;
            if (ownThread && method == "turn/started") _activeTurnId = p.GetProperty("turn").GetProperty("id").GetString();
            if (ownThread && method == "thread/tokenUsage/updated" && _usage.Observe(p)) EmitUsage();
            if (ownThread && method == "turn/completed" && p.GetProperty("turn").GetProperty("id").GetString() == _activeTurnId)
            {
                _turnStop?.Cancel();
                if (_work is { HasChanges: true, Finished: false })
                    Emit("turn/toolWarning", new { threadId = _threadId, message = "변경은 수행됐지만 완료 검증·프로젝트 기억 저장이 확인되지 않았습니다. 완료로 간주하지 마세요." });
            }
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
        _project = project; _tools = new ChatGptProjectTools(project);
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
                "Native shell/file operations are unavailable. Their read-only sandbox does not describe PixelTool editor permissions. " +
                "For requested Asset reads, writes, moves and renames, use pixeltool tools, which enforce this turn's edit toggle and approval mode. " +
                "If permission is uncertain, call pixeltool.get_asset_permissions; do not infer that the editor toggle failed from sandbox_mode. " +
                "After script changes reimport and fix reported errors. " +
                "Use focused line reads; preserve existing files and use exact patches. End with a short result/change summary; code changes are shown collapsed by the editor.";
            p["developerInstructions"] = p["developerInstructions"]!.GetValue<string>() + CodexWorkflowTools.Instructions;
            if (method == "thread/start")
            {
                var tools = ChatGptProjectTools.Definitions.Concat(CodexWorkflowTools.Definitions).Select(definition =>
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
            _newThread = method == "thread/start";
            return result;
        }
        if (method == "turn/start")
        {
            _allowEdits = p["sandboxPolicy"]?["type"]?.GetValue<string>() == "workspaceWrite";
            _confirmActions = p["confirmActions"]?.GetValue<bool>() ?? true;
            p.Remove("confirmActions");
            bool requirePlan = p["requirePlan"]?.GetValue<bool>() ?? false;
            p.Remove("requirePlan");
            _model = p["model"]?.GetValue<string>() ?? "";
            _activeTurnId = null;
            long? cursor = null;
            try { cursor = ReadEditorLogs?.Invoke(0, 1).GetProperty("latestCursor").GetInt64(); }
            catch (Exception) { /* Completion cannot claim success without a baseline. */ }
            _work = new CodexWorkSession(_project, cursor, requirePlan);
            _usage.BeginWork(_threadId, _newThread);
            _newThread = false;
            EmitUsage();
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
        bool held = false, mutating = false;
        CodexWorkSession? ownedWork = null;
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
            var work = _work ?? throw new InvalidOperationException("진행 중인 작업이 없습니다.");
            ownedWork = work;
            if (name == "delegate_project_task")
                return await DelegateAsync(args, work, token).ConfigureAwait(false);
            await _toolGate.WaitAsync(token).ConfigureAwait(false);
            held = true;
            token.ThrowIfCancellationRequested();
            if (CodexWorkSession.IsMutation(name)) { work.CheckMutation(name, args); mutating = true; }
            async Task<bool> ConfirmOperation(string path, string before, string after)
            {
                if (Approve == null) return false;
                var request = JsonSerializer.SerializeToElement(new { threadId = _threadId, turnId,
                    itemId = p.GetProperty("callId").GetString(), path, operation = name,
                    previousContent = before, proposedContent = after });
                return await Approve(name == "reimport_project" ? "item/commandExecution/requestApproval" : "item/fileChange/requestApproval", request)
                    .WaitAsync(token).ConfigureAwait(false);
            }
            if (name == "begin_project_work")
            {
                var plan = work.ValidatePlan(args);
                string answer = "간단 작업";
                if (plan.Mode == "planned")
                {
                    work.PrepareApproval();
                    if (AskChoice == null) throw new InvalidOperationException("계획 승인 화면을 사용할 수 없습니다.");
                    string question = $"작업 계획 승인\n{plan.Summary}\n이유: {plan.Reason}\n대상: {string.Join(", ", plan.Paths)}\n역할: 관리자(수정)" +
                        (plan.Roles.Length == 0 ? "" : ", " + string.Join(", ", plan.Roles) + "(읽기 전용)") + "\n" + plan.Details +
                        "\n기존 파일 수정 허용·작업별 승인 설정은 유지됩니다.";
                    answer = await AskChoice(question, new[] { "이 계획 승인", "계획 수정", "취소" }, token).WaitAsync(token).ConfigureAwait(false);
                    token.ThrowIfCancellationRequested();
                    if (answer != "이 계획 승인")
                        return new { success = false, contentItems = new[] { new { type = "inputText", text = JsonSerializer.Serialize(new { approved = false, answer }) } } };
                }
                work.AcceptPlan(plan);
                Emit("turn/teamStatus", new { threadId = _threadId, message = plan.Mode == "planned" ? "계획 승인됨 · 관리자 수정 / 필요한 보조 역할만 조회" : "간단 작업 · 관리자 단독 처리" });
                result = JsonSerializer.Serialize(new { approved = true, mode = plan.Mode, paths = plan.Paths, roles = plan.Roles });
            }
            else if (name == "complete_project_work")
            {
                if (!_allowEdits) throw new InvalidOperationException("파일 수정 허용이 꺼져 있어 Editor 작업 기억을 저장하지 않았습니다.");
                string summary = CodexWorkSession.Text(args, "summary", 500), reason = CodexWorkSession.Text(args, "reason", 500);
                if (!work.Finished && _confirmActions && !await ConfirmOperation(ProjectWorkLog.RelativePath,
                    "이전 작업 기억은 보존합니다.", summary + "\n이유: " + reason + "\n관찰한 변경 경로와 로그 검증 결과를 덧붙입니다.").ConfigureAwait(false))
                    throw new InvalidOperationException("작업 기억 저장을 승인하지 않았습니다. 기존 기록은 보존했습니다.");
                token.ThrowIfCancellationRequested();
                var saved = work.Complete(summary, reason, ReadEditorLogs, null);
                success = saved.Saved;
                result = JsonSerializer.Serialize(saved);
                Emit(saved.Saved ? "turn/teamStatus" : "turn/toolWarning", new { threadId = _threadId, message = saved.Saved ? "로그 확인 완료 · Editor/AIWorkLog.md에 작업 기억 저장" : saved.Message });
            }
            else if (name == "ask_choice")
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
                var imported = await (ReimportProject ?? throw new InvalidOperationException("리임포트 도구가 없습니다."))(token).ConfigureAwait(false);
                work.Reimported(imported);
                result = imported.GetRawText();
            }
            else
                result = await _tools!.InvokeAsync(name, args, _allowEdits, _confirmActions, ConfirmOperation, token, PickAsset,
                    (path, before, after) => Emit("item/completed", new { threadId = _threadId, turnId,
                        item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", path, before, after } }), CanDeleteAssetFile).ConfigureAwait(false);
            work.TrackResult(name, result);
            if (name is "write_project_file" or "patch_project_file" or "delete_asset_file" or "delete_asset_folder" or "move_asset" or "create_asset_folder" or "create_png_asset" or "copy_asset" or "import_asset")
                Emit("item/completed", new { threadId = _threadId, turnId, item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", result } });
        }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            success = false;
            result = "도구 작업 실패: " + ex.Message;
            if (!token.IsCancellationRequested) Emit("turn/toolWarning", new { threadId = _threadId, turnId, message = result });
        }
        finally { if (mutating) ownedWork?.EndMutation(); if (held) _toolGate.Release(); }
        return new { success, contentItems = new[] { new { type = "inputText", text = result } } };
    }

    private void EmitUsage() => Emit("turn/usage", new { threadId = _threadId, text = _usage.Snapshot().Text });

    private async Task<object> DelegateAsync(JsonElement args, CodexWorkSession work, CancellationToken token)
    {
        string role = CodexWorkSession.Text(args, "role", 30);
        string task = CodexWorkSession.Text(args, "task", 3000);
        string context = args.GetProperty("context").GetString() ?? "";
        if (context.Length > 6000) throw new InvalidOperationException("보조 작업 문맥은 6000자 이내여야 합니다.");
        work.StartWorker(role);
        string dispatch = CodexChatClient.SanitizeDiagnostic(task);
        Emit("turn/agentDispatch", new { threadId = _threadId, role, instruction = dispatch.Length > 1200 ? dispatch[..1200] + "…" : dispatch });
        try
        {
            var result = await CodexReadOnlyWorker.RunAsync(_project, _model, role, task, context,
                (id, label) => { if (!token.IsCancellationRequested && ReferenceEquals(_work, work)) { _usage.RegisterWorker(id, label); EmitUsage(); Emit("turn/teamStatus", new { threadId = _threadId, message = label + " · 읽기 전용 보조 작업 중" }); } },
                usage => { if (!token.IsCancellationRequested && ReferenceEquals(_work, work) && _usage.Observe(usage)) EmitUsage(); }, token).ConfigureAwait(false);
            if (!token.IsCancellationRequested && ReferenceEquals(_work, work)) Emit("turn/teamStatus", new { threadId = _threadId, message = role + " · " + result.Status + " · " + (int)result.Duration.TotalSeconds + "초" });
            return new { success = result.Status == "completed", contentItems = new[] { new { type = "inputText", text = JsonSerializer.Serialize(result) } } };
        }
        finally { work.EndWorker(); }
    }

    public async ValueTask DisposeAsync()
    {
        _stop.Cancel();
        _turnStop?.Cancel();
        await _server.DisposeAsync().ConfigureAwait(false);
    }
}
