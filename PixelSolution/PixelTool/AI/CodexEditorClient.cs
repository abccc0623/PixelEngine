using System.IO;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace PixelTool;

// Keeps editor-only operations on the existing permission-checked tool path.
internal sealed class CodexEditorClient : IAsyncDisposable
{
    // UI-only correlation metadata; never serialized into CLI requests or tool replies.
    internal string? LocalLogRequestId { get; set; }
    private string? _startingLogRequestId;
    private readonly System.Collections.Concurrent.ConcurrentDictionary<string, string> _logRequests = new();
    private readonly CodexChatClient _server = new();
    private readonly CancellationTokenSource _stop = new();
    private CancellationTokenSource? _turnStop;
    private ChatGptProjectTools? _tools;
    private string _threadId = "";
    private string _project = "", _model = "", _programmerModel = "", _plannerModel = "", _artistModel = "";
    private string? _programmerEffort, _plannerEffort, _artistEffort;
    private CodexProgrammer? _programmer, _planner;
    private string? _activeTurnId;
    private bool _newThread;
    private bool _startingTurn;
    private string? _interruptedTurnId;
    private readonly HashSet<string> _finishedTurns = new(StringComparer.Ordinal);
    private CodexWorkSession? _work;
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
    public Func<string, int, CancellationToken, Task<JsonElement>>? TestGameKey { get; set; }
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
                _ = CompleteTurnAsync(p.Clone(), _work);
                return;
            }
            Emit(method, p);
        };
        _server.Disconnected += details =>
        {
            try { _turnStop?.Cancel(); } catch (ObjectDisposedException) { }
            _ = StopProgrammerAsync();
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
                "For requested Asset reads, writes, moves and renames, use pixeltool tools, which enforce project paths and approved work scope. " +
                "If permission is uncertain, call pixeltool.get_asset_permissions; the CLI sandbox does not govern PixelTool project tools. " +
                "Run reimport and error checks when the user requests validation. " +
                "Use focused line reads; preserve existing files and use exact patches. Keep replies brief; ask required questions via ask_choice.";
            p["developerInstructions"] = p["developerInstructions"]!.GetValue<string>() + CodexWorkflowTools.Instructions;
            if (method == "thread/start")
            {
                var tools = ChatGptProjectTools.Definitions.Concat(CodexWorkflowTools.Definitions)
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
            _startingLogRequestId = LocalLogRequestId;
            using var requestLogScope = AiActionLog.Enter(_startingLogRequestId);
            _turnStop?.Cancel();
            await StopProgrammerAsync().ConfigureAwait(false);
            requestToken.ThrowIfCancellationRequested();
            lock (_finishedTurns)
            {
                _activeTurnId = _interruptedTurnId = null;
                _turnStop?.Dispose();
                _turnStop = CancellationTokenSource.CreateLinkedTokenSource(_stop.Token, requestToken);
                _startingTurn = true;
            }
            p.Remove("confirmActions");
            bool requirePlan = p["requirePlan"]?.GetValue<bool>() ?? false;
            p.Remove("requirePlan");
            _model = p["model"]?.GetValue<string>() ?? "";
            _programmerModel = p["programmerModel"]?.GetValue<string>() ?? "";
            _programmerEffort = p["programmerEffort"]?.GetValue<string>();
            _plannerModel = p["plannerModel"]?.GetValue<string>() ?? "";
            _plannerEffort = p["plannerEffort"]?.GetValue<string>();
            _artistModel = p["artistModel"]?.GetValue<string>() ?? "";
            _artistEffort = p["artistEffort"]?.GetValue<string>();
            p.Remove("programmerModel"); p.Remove("programmerEffort");
            p.Remove("plannerModel"); p.Remove("plannerEffort");
            p.Remove("artistModel"); p.Remove("artistEffort");
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
            // Linked cancellation also stops programmer/reviewer calls.
            if (_activeTurnId != null) await InterruptStartedTurnAsync(_activeTurnId).ConfigureAwait(false);
            return JsonSerializer.SerializeToElement(new { });
        }
        return await _server.RequestAsync(method, p).ConfigureAwait(false);
    }

    private async Task CompleteTurnAsync(JsonElement completed, CodexWorkSession? work)
    {
        // A write may have committed immediately before cancellation. Its receipt and
        // editor path refresh must be delivered before the UI enables a new request.
        try
        {
            await StopProgrammerAsync().ConfigureAwait(false);
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

    private async Task<object> InvokeToolCoreAsync(JsonElement p, bool programmer = false, CodexWorkSession? expectedWork = null, CancellationToken? programmerToken = null, string? workerLogRequestId = null)
    {
        using var logScope = AiActionLog.Enter(programmer ? workerLogRequestId : RequestFor(p));
        string name = p.TryGetProperty("tool", out var tool) ? tool.GetString() ?? "unknown" : "unknown";
        string target = p.TryGetProperty("arguments", out var args) ? AiActionLog.Target(args) : "";
        try
        {
            var reply = await InvokeToolOperationAsync(p, programmer, expectedWork, programmerToken).ConfigureAwait(false);
            var json = JsonSerializer.SerializeToElement(reply);
            bool success = json.TryGetProperty("success", out var ok) && ok.ValueKind == JsonValueKind.True;
            string result = json.GetRawText();
            if (success && json.TryGetProperty("contentItems", out var items) && items.ValueKind == JsonValueKind.Array)
                foreach (var item in items.EnumerateArray())
                    if (item.TryGetProperty("text", out var text) && text.ValueKind == JsonValueKind.String)
                    { result = text.GetString() ?? ""; break; }
            bool loggedSuccess = AiActionLog.Record((programmer ? "프로그래머 도구 " : "AI 도구 ") + name, target, result, success);
            Emit("turn/toolOutcome", new { threadId = _threadId, turnId = _activeTurnId, tool = name, success = loggedSuccess });
            return reply;
        }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            AiActionLog.Failed("AI 도구 " + name, target, ex);
            throw;
        }
    }

    private async Task<object> InvokeToolOperationAsync(JsonElement p, bool programmer = false, CodexWorkSession? expectedWork = null, CancellationToken? programmerToken = null)
    {
        string result;
        bool success = true;
        bool held = false, mutating = false;
        CodexWorkSession? ownedWork = null;
        string name = p.GetProperty("tool").GetString()!;
        string turnId = programmer ? _activeTurnId ?? "" : p.GetProperty("turnId").GetString()!;
        var token = programmerToken ?? _turnStop?.Token ?? _stop.Token;
        try
        {
            token.ThrowIfCancellationRequested();
            if ((!programmer && p.GetProperty("threadId").GetString() != _threadId) ||
                !p.TryGetProperty("namespace", out var ns) || ns.GetString() != "pixeltool")
                throw new InvalidOperationException("현재 대화의 에디터 도구만 호출할 수 있습니다.");
            var args = p.GetProperty("arguments");
            if (_activeTurnId != turnId) throw new InvalidOperationException("현재 요청의 도구만 호출할 수 있습니다.");
            var work = _work ?? throw new InvalidOperationException("진행 중인 작업이 없습니다.");
            if (programmer && (!ReferenceEquals(work, expectedWork) || !(CodexProgrammer.ReadTools.Contains(name) || CodexProgrammer.EditTools.Contains(name))))
                throw new InvalidOperationException("이 플머의 작업 범위 또는 도구가 현재 요청과 다릅니다.");
            ownedWork = work;
            if (name == "planner_task")
            {
                return await PlannerAsync(args, work, token).ConfigureAwait(false);
            }
            if (name == "programmer_task")
            {
                return await ProgrammerAsync(args, work, token).ConfigureAwait(false);
            }
            if (name == "delegate_project_task")
            {
                return await DelegateAsync(args, work, token).ConfigureAwait(false);
            }
            await _toolGate.WaitAsync(token).ConfigureAwait(false);
            held = true;
            token.ThrowIfCancellationRequested();
            if (!ReferenceEquals(_work, work) || _activeTurnId != turnId) throw new InvalidOperationException("현재 요청이 종료되었거나 바뀌었습니다.");
            if (CodexWorkSession.IsMutation(name)) { work.CheckMutation(name, args, programmer); mutating = true; }
            if (name == "begin_project_work")
            {
                string mode = args.GetProperty("mode").GetString() ?? "";
                bool planned = mode is "middle" or "planned";
                bool separatePlanner = planned && !string.IsNullOrEmpty(args.GetProperty("proposalId").GetString());
                var plan = separatePlanner ? work.PlanForReview(args) : planned ? work.ManagerPlanForReview(args) : work.ValidatePlan(args);
                bool approvalReused = planned ? work.IsPlanApproved(plan) : work.IsScopeApproved(plan);
                if (plan.Mode == "scoped" && !approvalReused) work.PrepareScopeApproval(plan);

                token.ThrowIfCancellationRequested();
                plan = work.AcceptPlan(plan);
                if (!approvalReused) Emit("turn/teamStatus", new { threadId = _threadId, message = plan.Mode == "planned" ? "상세 계획 등록됨" : plan.Mode == "middle" ? "계획 등록됨" : "작업 범위 등록됨" });
                result = JsonSerializer.Serialize(new { approved = true, approvalReused, mode = plan.Mode,
                    proposalId = plan.ProposalId, paths = plan.Paths, roles = plan.Roles,
                    next = (plan.Roles.Contains("programmer") ? "Continue the SAME programmer with implement. " : "Perform the approved work directly with hash-checked project tools. ") +
                        "Use the registered scope for the same work. Path, hash and unsaved-buffer checks still apply; no approval dialog is shown." });
            }
            else if (name == "complete_project_work")
            {
                string summary = CodexWorkSession.Text(args, "summary", 500), reason = CodexWorkSession.Text(args, "reason", 500);
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
                    args.GetProperty("afterCursor").GetInt64(), Math.Clamp(args.GetProperty("limit").GetInt32(), 1, 100)).GetRawText();
            else if (name == "clear_editor_logs")
            {
                if (ClearEditorLogs == null) throw new InvalidOperationException("로그 삭제 도구가 연결되지 않았습니다.");
                long cursor = await ClearEditorLogs().WaitAsync(token).ConfigureAwait(false);
                result = JsonSerializer.Serialize(new { cleared = true, afterCursor = cursor });
            }
            else if (name == "test_game_key")
                result = (await (TestGameKey ?? throw new InvalidOperationException("게임 입력 도구가 연결되지 않았습니다."))(
                    args.GetProperty("key").GetString()!, args.GetProperty("frames").GetInt32(), token).WaitAsync(token).ConfigureAwait(false)).GetRawText();
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
                work.Reimported(imported);
                result = imported.GetRawText();
            }
            else
                result = await _tools!.InvokeAsync(name, args, _allowEdits, false, (_, _, _) => Task.FromResult(false), token, PickAsset,
                    (path, before, after) => Emit("item/completed", new { threadId = _threadId, turnId,
                        item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", path, before, after } }), CanDeleteAssetFile).ConfigureAwait(false);
            work.TrackResult(name, result);
            if (name is "write_project_file" or "patch_project_file" or "delete_asset_file" or "delete_asset_folder" or "move_asset" or "create_asset_folder" or "create_png_asset" or "copy_asset" or "import_asset")
                Emit("item/completed", new { threadId = _threadId, turnId, item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", result } });
        }
        catch (Exception ex) when (ex is not OutOfMemoryException)
        {
            success = false;
            result = ex.Message == ProjectFileAccess.AssetMutationDenied ? ex.Message : "도구 작업 실패: " + ex.Message;
            if (!token.IsCancellationRequested) Emit("turn/toolWarning", new { threadId = _threadId, turnId, message = result });
        }
        finally { if (mutating) ownedWork?.EndMutation(); if (held) _toolGate.Release(); }
        return new { success, contentItems = new[] { new { type = "inputText", text = result } } };
    }

    private void ObserveUsage(JsonElement usage)
    {
        _usage.Observe(usage);
        EmitUsage();
    }

    private void EmitUsage() => Emit("turn/usage", new { threadId = _threadId, text = _usage.Snapshot().Text });

    private async Task<object> ProgrammerAsync(JsonElement args, CodexWorkSession work, CancellationToken token)
    {
        string? logRequestId = AiActionLog.CurrentRequestId;
        string phase = CodexWorkSession.Text(args, "phase", 20);
        string task = CodexWorkSession.Text(args, "task", 3000);
        string context = args.GetProperty("context").GetString() ?? "";
        if (context.Length > 6000 || string.IsNullOrWhiteSpace(_programmerModel))
            throw new InvalidOperationException("플머 모델을 선택하고 6000자 이내의 문맥을 전달하세요.");
        work.StartProgrammer(phase);
        bool completed = false;
        long beforeTokens = _usage.Snapshot().Workers.FirstOrDefault(w => w.Label == "programmer")?.WorkTokens ?? 0;
        try
        {
            var programmer = _programmer ??= new CodexProgrammer(usage =>
            {
                if (!token.IsCancellationRequested && ReferenceEquals(_work, work)) ObserveUsage(usage);
            });
            string instruction = CodexChatClient.SanitizeDiagnostic(task);
            Emit("turn/agentDispatch", new { threadId = _threadId, role = "programmer / " + phase + " / " + _programmerModel,
                message = CodexWorkflowTools.DispatchMessage("programmer", phase, instruction),
                instruction = instruction.Length > 1200 ? instruction[..1200] + "…" : instruction });
            var result = await programmer.RunAsync(_project, _programmerModel, _programmerEffort, phase, task, context,
                (call, workerToken) => InvokeToolCoreAsync(call, true, work, workerToken, logRequestId),
                id => { if (!token.IsCancellationRequested && ReferenceEquals(_work, work)) { _usage.RegisterWorker(id, "programmer"); EmitUsage(); } }, token).ConfigureAwait(false);
            completed = result.Status == "completed";
            if (!token.IsCancellationRequested && ReferenceEquals(_work, work))
            {
                long? afterTokens = _usage.Snapshot().Workers.FirstOrDefault(w => w.ThreadId == result.ThreadId)?.WorkTokens;
                string measured = afterTokens is long after && after >= beforeTokens
                    ? " · 이번 단계 " + (after - beforeTokens).ToString("N0") + "토큰" : " · 토큰 미측정";
                Emit("turn/teamStatus", new { threadId = _threadId, message = "programmer · " + result.Model + " · " + phase + " · " + result.Status + " · " + (int)result.Duration.TotalSeconds + "초 · 도구 " + result.ToolCalls + "회" + measured });
            }
            return new { success = completed, contentItems = new[] { new { type = "inputText", text = JsonSerializer.Serialize(result) } } };
        }
        finally { work.EndProgrammer(phase, completed); }
    }

    private static object ToolReply(bool success, object result) => new
        { success, contentItems = new[] { new { type = "inputText", text = JsonSerializer.Serialize(result) } } };

    private async Task<object> PlannerAsync(JsonElement args, CodexWorkSession work, CancellationToken token)
    {
        string phase = CodexWorkSession.Text(args, "phase", 20);
        string task = args.GetProperty("task").GetString() ?? "", context = args.GetProperty("context").GetString() ?? "";
        if (string.IsNullOrWhiteSpace(_plannerModel) || task.Length > 3000 || context.Length > 6000 ||
            (phase == "draft" && (string.IsNullOrWhiteSpace(task) || string.IsNullOrWhiteSpace(context))) ||
            (phase == "revise" && (task.Length != 0 || context.Length != 0 || _planner == null)))
            throw new InvalidOperationException("초안에는 관리자 기능·근거 목록과 요청을 전달하세요. 수정은 같은 기획자에게 빈 task/context로 요청하세요.");
        work.StartPlanner(phase);
        try
        {
            var planner = _planner ??= new CodexProgrammer(usage =>
            {
                if (!token.IsCancellationRequested && ReferenceEquals(_work, work)) ObserveUsage(usage);
            }, planning: true);
            if (phase == "revise")
            {
                task = "REVISION: Replan exactly the requested numbered sections using the user's feedback; return only their sections JSON.";
                context = work.Proposal!.RevisionContext(work.PendingRevision!);
            }
            else task = "DRAFT: " + task;
            Emit("turn/agentDispatch", new { threadId = _threadId, role = "planner / " + phase + " / " + _plannerModel,
                message = CodexWorkflowTools.DispatchMessage("planner", phase, ""),
                instruction = phase == "revise" ? "사용자가 지정한 기획 항목만 수정" : "엔진 지원 근거를 바탕으로 기능별 기획안 작성" });
            string? logRequestId = AiActionLog.CurrentRequestId;
            var result = await planner.RunAsync(_project, _plannerModel, _plannerEffort, "analyze", task, context,
                async (call, workerToken) =>
                {
                    using var logScope = AiActionLog.Enter(logRequestId);
                    workerToken.ThrowIfCancellationRequested();
                    if (!ReferenceEquals(_work, work) || !CodexProgrammer.ReadTools.Contains(call.GetProperty("tool").GetString()!))
                        return ToolReply(false, new { error = "Planner is read-only and must belong to this request." });
                    string name = call.GetProperty("tool").GetString()!;
                    string target = AiActionLog.Target(call.GetProperty("arguments"));
                    try
                    {
                        string output = await _tools!.InvokeAsync(name, call.GetProperty("arguments"),
                            allowEdits: false, confirmChanges: true, (_, _, _) => Task.FromResult(false), workerToken).ConfigureAwait(false);
                        bool success = AiActionLog.Record("기획자 도구 " + name, target, output);
                        return new { success, contentItems = new[] { new { type = "inputText", text = output } } };
                    }
                    catch (Exception ex) when (ex is not OutOfMemoryException)
                    {
                        AiActionLog.Failed("기획자 도구 " + name, target, ex);
                        throw;
                    }
                },
                id => { if (!token.IsCancellationRequested && ReferenceEquals(_work, work)) { _usage.RegisterWorker(id, "planner"); EmitUsage(); } },
                token).ConfigureAwait(false);
            token.ThrowIfCancellationRequested();
            if (!ReferenceEquals(_work, work)) throw new InvalidOperationException("기획 요청이 종료되었거나 바뀌었습니다.");
            if (result.Status != "completed") return ToolReply(false, new { role = "planner", result.ThreadId, result.Status, result.Summary });
            var proposal = work.StoreProposal(result.Summary, phase);
            Emit("turn/teamStatus", new { threadId = _threadId, message = "기획안 작성 완료 · 관리자 검토 중" });
            return ToolReply(true, new { role = "planner", threadId = result.ThreadId, proposalId = proposal.Id,
                summary = proposal.Summary, reason = proposal.Reason, paths = proposal.Paths, roles = proposal.Roles,
                proposal = proposal.Details, next = "Review feasibility/scope, then begin_project_work planned using this proposalId and managerReview." });
        }
        finally { work.EndPlanner(); }
    }
    private async Task StopProgrammerAsync()
    {
        var programmer = Interlocked.Exchange(ref _programmer, null);
        var planner = Interlocked.Exchange(ref _planner, null);
        try { if (programmer != null) await programmer.DisposeAsync().ConfigureAwait(false); }
        finally { if (planner != null) await planner.DisposeAsync().ConfigureAwait(false); }
    }
    private async Task<object> DelegateAsync(JsonElement args, CodexWorkSession work, CancellationToken token)
    {
        string role = CodexWorkSession.Text(args, "role", 30);
        string task = CodexWorkSession.Text(args, "task", 3000);
        string context = args.GetProperty("context").GetString() ?? "";
        if (context.Length > 6000) throw new InvalidOperationException("보조 작업 문맥은 6000자 이내여야 합니다.");
        work.StartWorker(role);
        string dispatch = CodexChatClient.SanitizeDiagnostic(task);
        Emit("turn/agentDispatch", new { threadId = _threadId, role,
            message = CodexWorkflowTools.DispatchMessage(role, "", dispatch),
            instruction = dispatch.Length > 1200 ? dispatch[..1200] + "…" : dispatch });
        try
        {
            var result = await CodexReadOnlyWorker.RunAsync(_project, role == "artist" ? _artistModel : _model,
                role == "artist" ? _artistEffort : null, role, task, context,
                (id, label) => { if (!token.IsCancellationRequested && ReferenceEquals(_work, work)) { _usage.RegisterWorker(id, label); EmitUsage(); Emit("turn/teamStatus", new { threadId = _threadId, message = label + " · 읽기 전용 보조 작업 중" }); } },
                usage => { if (!token.IsCancellationRequested && ReferenceEquals(_work, work)) ObserveUsage(usage); }, token).ConfigureAwait(false);
            if (!token.IsCancellationRequested && ReferenceEquals(_work, work)) Emit("turn/teamStatus", new { threadId = _threadId, message = role + " · " + result.Status + " · " + (int)result.Duration.TotalSeconds + "초" });
            return new { success = result.Status == "completed", contentItems = new[] { new { type = "inputText", text = JsonSerializer.Serialize(result) } } };
        }
        finally { work.EndWorker(); }
    }

    public async ValueTask DisposeAsync()
    {
        _stop.Cancel();
        _turnStop?.Cancel();
        await StopProgrammerAsync().ConfigureAwait(false);
        await _server.DisposeAsync().ConfigureAwait(false);
    }
}
