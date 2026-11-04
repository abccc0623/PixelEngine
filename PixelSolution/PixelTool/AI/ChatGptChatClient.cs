using System.IO;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Net.Http.Json;
using System.Text;
using System.Text.Json;

namespace PixelTool;

// Small compatibility adapter for the existing editor UI, not a Codex process.
internal sealed class ChatGptChatClient : IAsyncDisposable
{
    private readonly ChatGptSignIn _auth = new();
    private readonly HttpClient _http = new(new HttpClientHandler { AllowAutoRedirect = false }) { Timeout = TimeSpan.FromMinutes(5) };
    private readonly CancellationTokenSource _stop = new();
    private CancellationTokenSource? _turnStop;
    private Task? _turn;
    private string _project = "", _threadId = "", _instructions = "", _account = "";
    private List<JsonElement> _input = new();
    public event Action<string, JsonElement>? Notification;
    public event Action? Disconnected;
    public Func<string, JsonElement, Task<bool>>? Approve { get; set; }
    public Func<Task<ChatGptProjectTools.ImportedAsset?>>? PickAsset { get; set; }
    public Func<CancellationToken, Task<JsonElement>>? ReimportProject { get; set; }
    public Func<long, int, JsonElement>? ReadEditorLogs { get; set; }
    public Func<string, string[], CancellationToken, Task<string>>? AskChoice { get; set; }

    internal ChatGptChatClient()
    {
        _auth.LoginCompleted += success => Emit("account/login/completed", new { success, message = _auth.LoginError });
    }

    public Task ConnectAsync(string project)
    {
        _project = Path.GetFullPath(project);
        return Task.CompletedTask;
    }

    public async Task<JsonElement> RequestAsync(string method, object parameters)
    {
        var p = JsonSerializer.SerializeToElement(parameters);
        switch (method)
        {
            case "account/list":
                return JsonSerializer.SerializeToElement(new { data = _auth.Accounts(), selected = _auth.CurrentKey });
            case "account/select":
                if (_turn is { IsCompleted: false }) throw new InvalidOperationException("응답 중에는 계정을 바꿀 수 없습니다.");
                _auth.SelectAccount(p.GetProperty("id").GetString()!);
                return JsonSerializer.SerializeToElement(new { });
            case "account/read":
                if (_auth.Current == null) return JsonSerializer.SerializeToElement(new { account = (object?)null });
                return JsonSerializer.SerializeToElement(new { account = new { type = "chatgpt", planType = _auth.Current.Email } });
            case "account/login/start":
                bool add = p.TryGetProperty("newAccount", out var newAccount) && newAccount.GetBoolean();
                return JsonSerializer.SerializeToElement(new { authUrl = await _auth.BeginLoginAsync(add).ConfigureAwait(false) });
            case "model/list":
                using (var response = await SendAsync(HttpMethod.Get, "models", null, _stop.Token).ConfigureAwait(false))
                using (var document = JsonDocument.Parse(await response.Content.ReadAsStringAsync(_stop.Token).ConfigureAwait(false)))
                {
                    var models = document.RootElement.GetProperty("models").EnumerateArray()
                        .Where(m => m.TryGetProperty("visibility", out var visible) && visible.GetString() == "list")
                        .Select(m => new { model = m.GetProperty("slug").GetString(), displayName = m.GetProperty("display_name").GetString(), isDefault = false }).ToArray();
                    return JsonSerializer.SerializeToElement(new { data = models, nextCursor = (string?)null });
                }
            case "thread/start":
            case "thread/resume":
                _instructions = p.GetProperty("developerInstructions").GetString() ?? "";
                _account = AccountIdentity();
                _threadId = method == "thread/resume" ? p.GetProperty("threadId").GetString()! : "chatgpt-" + Guid.NewGuid().ToString("N");
                string path = ThreadPath();
                if (method == "thread/resume")
                {
                    var saved = JsonSerializer.Deserialize<SavedThread>(await File.ReadAllTextAsync(path, _stop.Token).ConfigureAwait(false))
                        ?? throw new InvalidDataException("저장된 대화가 잘못되었습니다.");
                    if (saved.Project != _project || saved.Account != _account) throw new InvalidDataException("이 대화는 다른 프로젝트 또는 ChatGPT 계정의 대화입니다. 새 대화를 시작하세요.");
                    _input = saved.Input;
                }
                else _input = new();
                return JsonSerializer.SerializeToElement(new { thread = new { id = _threadId, cwd = _project } });
            case "turn/start":
                if (_turn is { IsCompleted: false }) throw new InvalidOperationException("아직 응답 중입니다.");
                if (p.GetProperty("threadId").GetString() != _threadId || AccountIdentity() != _account) throw new InvalidOperationException("대화 또는 계정이 변경되었습니다. 새 대화를 시작하세요.");
                _turnStop?.Dispose();
                _turnStop = CancellationTokenSource.CreateLinkedTokenSource(_stop.Token);
                _turnStop.CancelAfter(TimeSpan.FromMinutes(10));
                string turnId = Guid.NewGuid().ToString("N");
                bool edits = p.GetProperty("sandboxPolicy").GetProperty("type").GetString() == "workspaceWrite";
                bool confirmActions = !p.TryGetProperty("confirmActions", out var confirm) || confirm.GetBoolean();
                var input = p.GetProperty("input").EnumerateArray().Select(i => i.GetProperty("text").GetString()).ToArray();
                string model = p.GetProperty("model").GetString()!;
                // Freeze data before running the background stream, so UI selection changes cannot alter this turn.
                var cancel = _turnStop.Token;
                _turn = Task.Run(() => RunTurnAsync(turnId, model, string.Join("\n\n", input), edits, confirmActions, cancel));
                return JsonSerializer.SerializeToElement(new { turn = new { id = turnId } });
            case "turn/interrupt":
                _turnStop?.Cancel();
                return JsonSerializer.SerializeToElement(new { });
            default: throw new NotSupportedException("직접 연결에서 지원하지 않는 요청입니다: " + method);
        }
    }

    private async Task RunTurnAsync(string turnId, string model, string text, bool allowEdits, bool confirmActions, CancellationToken token)
    {
        string status = "failed";
        object? error = null;
        try
        {
            Emit("turn/started", new { threadId = _threadId, turn = new { id = turnId } });
            var pending = new List<JsonElement>(_input) { JsonSerializer.SerializeToElement(new { role = "user", content = text }) };
            var tools = new ChatGptProjectTools(_project);
            for (int round = 0; round < 16; round++)
            {
                token.ThrowIfCancellationRequested();
                using var response = await SendAsync(HttpMethod.Post, "responses", new
                {
                    model, store = false, stream = true, include = new[] { "reasoning.encrypted_content" },
                    instructions = _instructions + "\nCurrent project: " + _project +
                        "\nUse the provided project tools to inspect files. Read Asset/main.lua first when tracing execution. " +
                        "Project tools can list/read/write text, create Asset folders, inspect/copy existing assets, import a user-picked external asset, and draw simple PNGs. " +
                        "For a requested simple placeholder or pixel graphic, use create_png_asset, not an unsupported-format excuse. SVG text can be saved but engine SVG rendering support must not be assumed; prefer PNG for textures. " +
                        "PNG drawing is procedural, not an image-generation AI. Reimport and editor-log tools are available: after script changes use reimport_project and inspect its logs, then read_editor_logs if more logs are needed. " +
                        "Reimport resets runtime; it is not a build or a new game/editor process. No arbitrary command execution, deletion, network, build or game launch tool exists. " +
                        "Never claim those actions were performed. Ask the user to perform them when needed. " +
                        CommandExecutionPreference.ForTurn(confirmActions) +
                        "\nThis is the current editor setting, including for file/list/read tools. Previous assistant promises to always ask do not override this setting. " +
                        (allowEdits ? "Project file edits are enabled for this turn." : "Read-only mode: do not request file writes."),
                    input = pending,
                    tools = new[] { new { type = "namespace", name = "pixeltool", description = "Approved local project text operations only.", tools = ChatGptProjectTools.Definitions } },
                    parallel_tool_calls = false
                }, token).ConfigureAwait(false);
                var output = await ReadStreamAsync(response, turnId, token).ConfigureAwait(false);
                pending.AddRange(output);
                var calls = output.Where(o => o.GetProperty("type").GetString() == "function_call").ToArray();
                if (calls.Length == 0)
                {
                    _input = pending;
                    await SaveThreadAsync(token).ConfigureAwait(false);
                    status = "completed";
                    return;
                }
                foreach (var call in calls)
                {
                    token.ThrowIfCancellationRequested();
                    string result;
                    try
                    {
                        using var args = JsonDocument.Parse(call.GetProperty("arguments").GetString()!);
                        string name = call.GetProperty("name").GetString()!;
                        if (call.TryGetProperty("namespace", out var ns) && ns.GetString() != "pixeltool")
                            throw new InvalidOperationException("지원하지 않는 도구 네임스페이스입니다.");
                        async Task<bool> ConfirmOperation(string path, string before, string after)
                        {
                            string id = Guid.NewGuid().ToString("N");
                            var request = JsonSerializer.SerializeToElement(new { threadId = _threadId, turnId, itemId = id,
                                path, previousContent = before, proposedContent = after });
                            string approvalMethod = name == "reimport_project" ? "item/commandExecution/requestApproval" : "item/fileChange/requestApproval";
                            if (Approve == null || !await Approve(approvalMethod, request).WaitAsync(token).ConfigureAwait(false)) return false;
                            return !token.IsCancellationRequested;
                        }
                        if (name == "ask_choice")
                        {
                            string question = args.RootElement.GetProperty("question").GetString() ?? "";
                            string[] options = args.RootElement.GetProperty("options").EnumerateArray()
                                .Select(option => option.GetString() ?? "").ToArray();
                            if (string.IsNullOrWhiteSpace(question) || question.Length > 500 || options.Length is < 2 or > 5 ||
                                options.Any(option => string.IsNullOrWhiteSpace(option) || option.Length > 150) ||
                                options.Distinct(StringComparer.OrdinalIgnoreCase).Count() != options.Length)
                                throw new InvalidOperationException("질문과 서로 다른 선택지 2~5개가 필요합니다.");
                            if (AskChoice == null) throw new InvalidOperationException("선택형 질문 UI가 연결되지 않았습니다.");
                            _turnStop?.CancelAfter(Timeout.InfiniteTimeSpan);
                            string answer;
                            try { answer = await AskChoice(question, options, token).WaitAsync(token).ConfigureAwait(false); }
                            finally { _turnStop?.CancelAfter(TimeSpan.FromMinutes(10)); }
                            token.ThrowIfCancellationRequested();
                            result = JsonSerializer.Serialize(new { question, answer });
                        }
                        else if (name == "read_editor_logs")
                        {
                            if (ReadEditorLogs == null) throw new InvalidOperationException("로그 조회 도구가 연결되지 않았습니다.");
                            result = ReadEditorLogs(args.RootElement.GetProperty("afterCursor").GetInt64(), args.RootElement.GetProperty("limit").GetInt32()).GetRawText();
                        }
                        else if (name == "reimport_project")
                        {
                            if (!allowEdits) result = "작업 허용이 꺼져 있어 리임포트하지 않았습니다. 프로젝트 파일 수정 허용을 먼저 켜 주세요.";
                            else if (ReimportProject == null) result = "에디터 리임포트 도구가 연결되지 않았습니다.";
                            else if (confirmActions && !await ConfirmOperation("Asset/main.lua", "", "리임포트: 현재 런타임 상태를 초기화하고 메인부터 다시 읽습니다.").ConfigureAwait(false))
                                result = "리임포트를 승인하지 않았습니다.";
                            else result = (await ReimportProject(token).ConfigureAwait(false)).GetRawText();
                        }
                        else result = await tools.InvokeAsync(name, args.RootElement, allowEdits, confirmActions, ConfirmOperation, token, PickAsset,
                            (path, before, after) => Emit("item/completed", new { threadId = _threadId, turnId,
                                item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", path, before, after } })).ConfigureAwait(false);
                        if (name is "write_project_file" or "create_asset_folder" or "create_png_asset" or "copy_asset" or "import_asset")
                            Emit("item/completed", new { threadId = _threadId, turnId, item = new { id = Guid.NewGuid().ToString("N"), type = "fileChange", result } });
                    }
                    catch (OperationCanceledException) { throw; }
                    catch (Exception ex) when (ex is IOException or UnauthorizedAccessException or InvalidOperationException or ArgumentException or JsonException or KeyNotFoundException or FormatException or OverflowException)
                    {
                        result = "Project tool refused or failed: " + ex.GetType().Name + ". No success is assumed. Recheck the relative path, file size, hash and edit permission.";
                    }
                    pending.Add(JsonSerializer.SerializeToElement(new { type = "function_call_output", call_id = call.GetProperty("call_id").GetString(), output = result }));
                }
            }
            throw new InvalidOperationException("도구 호출 횟수 제한에 도달했습니다. 요청을 나누어 진행하세요.");
        }
        catch (OperationCanceledException) { status = "interrupted"; }
        catch (Exception ex)
        {
            error = new { message = ex is InvalidOperationException or InvalidDataException ? ex.Message : "ChatGPT 응답 처리에 실패했습니다. 네트워크와 계정 연결을 확인하세요." };
        }
        finally
        {
            if (!_stop.IsCancellationRequested) Emit("turn/completed", new { threadId = _threadId, turn = new { id = turnId, status, error } });
        }
    }

    private async Task<List<JsonElement>> ReadStreamAsync(HttpResponseMessage response, string turnId, CancellationToken token)
    {
        using var stream = await response.Content.ReadAsStreamAsync(token).ConfigureAwait(false);
        using var reader = new StreamReader(stream);
        var data = new StringBuilder();
        var output = new List<JsonElement>();
        bool completed = false;
        while (await reader.ReadLineAsync(token).ConfigureAwait(false) is { } line)
        {
            if (line.StartsWith("data:")) data.AppendLine(line[5..].TrimStart());
            if (data.Length > 2 * 1024 * 1024) throw new InvalidDataException("ChatGPT 응답 이벤트가 너무 큽니다.");
            if (line.Length != 0 || data.Length == 0) continue;
            string json = data.ToString(); data.Clear();
            if (json.Trim() == "[DONE]") break;
            using var document = JsonDocument.Parse(json);
            var e = document.RootElement;
            string? type = e.GetProperty("type").GetString();
            if (type == "response.output_text.delta")
                Emit("item/agentMessage/delta", new { threadId = _threadId, turnId,
                    itemId = e.GetProperty("item_id").GetString(), delta = e.GetProperty("delta").GetString() });
            if (type == "response.output_item.done")
            {
                var item = e.GetProperty("item").Clone();
                output.Add(item);
                if (item.GetProperty("type").GetString() == "message")
                {
                    string message = string.Join("", item.GetProperty("content").EnumerateArray().Select(c =>
                        c.TryGetProperty("text", out var t) ? t.GetString() : c.TryGetProperty("refusal", out var r) ? r.GetString() : ""));
                    Emit("item/completed", new { threadId = _threadId, turnId, item = new { id = item.GetProperty("id").GetString(), type = "agentMessage", text = message } });
                }
            }
            if (type == "response.completed") { completed = true; break; }
            if (type is "response.failed" or "response.incomplete" or "error")
            {
                string code = "unknown";
                JsonElement details = e.TryGetProperty("response", out var envelope) ? envelope : e;
                if (details.TryGetProperty("error", out var apiError) && apiError.ValueKind == JsonValueKind.Object && apiError.TryGetProperty("code", out var c)) code = c.GetString() ?? code;
                // Only known-safe error codes, never raw response bodies or tokens.
                if (code.Length > 100 || code.Any(ch => !(char.IsAsciiLetterOrDigit(ch) || ch == '_'))) code = "unknown";
                throw new InvalidOperationException("ChatGPT 응답이 완료되지 않았습니다: " + code + ". 요금제 사용 권한·한도 또는 요청 내용을 확인하세요.");
            }
        }
        if (!completed) throw new InvalidDataException("완료 이벤트 없이 연결이 끝났습니다. 실행 성공으로 처리하지 않았습니다.");
        return output;
    }

    private async Task<HttpResponseMessage> SendAsync(HttpMethod method, string endpoint, object? body, CancellationToken token)
    {
        using var request = new HttpRequestMessage(method, "https://api.openai.com/v1/" + endpoint);
        request.Headers.Authorization = new AuthenticationHeaderValue("Bearer", await _auth.AccessTokenAsync(token).ConfigureAwait(false));
        if (body != null) request.Content = JsonContent.Create(body);
        var response = await _http.SendAsync(request, HttpCompletionOption.ResponseHeadersRead, token).ConfigureAwait(false);
        if (response.IsSuccessStatusCode) return response;
        int status = (int)response.StatusCode;
        response.Dispose();
        throw new InvalidOperationException($"ChatGPT 요청 실패 (HTTP {status}). 계정 연결·사용 권한·한도를 확인하세요. 자동 재시도하지 않았습니다.");
    }

    private sealed class SavedThread
    {
        public string Project { get; set; } = "";
        public string Account { get; set; } = "";
        public List<JsonElement> Input { get; set; } = new();
        public SavedThread() { }
    }
    private string AccountIdentity() => _auth.Current?.ClientId + ":" + _auth.Current?.Subject;
    private string ThreadPath()
    {
        if (!_threadId.StartsWith("chatgpt-") || !Guid.TryParseExact(_threadId[8..], "N", out _))
            throw new InvalidDataException("직접 연결용 대화가 아닙니다. 새 대화를 시작하세요.");
        return CodexChatHistory.PathFor(_project) + "." + _threadId + ".json";
    }
    private async Task SaveThreadAsync(CancellationToken token)
    {
        string path = ThreadPath();
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        string temporary = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
        await File.WriteAllTextAsync(temporary, JsonSerializer.Serialize(new SavedThread { Project = _project, Account = _account, Input = _input }), token).ConfigureAwait(false);
        File.Move(temporary, path, true);
    }
    private void Emit(string name, object parameters) => Notification?.Invoke(name, JsonSerializer.SerializeToElement(parameters));
    public ValueTask DisposeAsync()
    {
        _stop.Cancel(); _turnStop?.Cancel(); _auth.Dispose(); _http.Dispose();
        // No UI callback, HTTP stream or approval task is synchronously awaited on close.
        return ValueTask.CompletedTask;
    }
}
