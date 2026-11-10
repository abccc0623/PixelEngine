using System.Collections.Concurrent;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

namespace PixelTool;

// JSONL over stdio, using the public Codex app-server protocol. Never read auth.json.
internal sealed class CodexChatClient : IAsyncDisposable
{
    private readonly ConcurrentDictionary<long, TaskCompletionSource<JsonElement>> _pending = new();
    private readonly SemaphoreSlim _writeLock = new(1, 1);
    private readonly CancellationTokenSource _transportStop = new();
    private Process? _process;
    private Task? _reader;
    private Task? _stderr;
    private long _nextId;
    private bool _closing;
    private readonly object _diagnosticLock = new();
    private readonly Queue<string> _errorTail = new();
    private Task<string>? _disconnectReport;
    private string? _transportFailure;
    public event Action<string, JsonElement>? Notification;
    public event Action<string>? Disconnected;
    public Func<string, JsonElement, Task<bool>>? Approve { get; set; }
    public Func<JsonElement, Task<object>>? CallTool { get; set; }

    public async Task ConnectAsync(string projectPath)
    {
        var start = ArtCodexCli.Command(projectPath, "app-server");
        // Native tools stay read-only; mutations use the editor's permission-checked tools.
        foreach (string value in new[] { "model_provider=\"openai\"", "forced_login_method=\"chatgpt\"",
            "features.shell_tool=false", "features.apply_patch_freeform=false", "features.codex_hooks=false",
            "features.multi_agent=false", "features.multi_agent_v2=false",
            // Keep human decisions as pending protocol calls, outside code-mode exec/wait polling.
            "features.code_mode.direct_only_tool_namespaces=[\"pixeltool\"]",
            "sandbox_mode=\"read-only\"", "approval_policy=\"never\"", "mcp_servers={}" })
        {
            start.ArgumentList.Add("-c");
            start.ArgumentList.Add(value);
        }
        StartTransport(start);
        try
        {
            await RequestAsync("initialize", new { clientInfo = new { name = "pixeltool", title = "AI 자비스", version = "0.1.0" },
                capabilities = new { experimentalApi = true } }).ConfigureAwait(false);
            await WriteAsync(new { method = "initialized", @params = new { } }).ConfigureAwait(false);
        }
        catch { await DisposeAsync().ConfigureAwait(false); throw; }
    }

    public async Task ConnectAsync(string executable, string projectPath, bool connectLocalMcp)
    {
        if (_process != null) throw new InvalidOperationException("Codex is already connected.");
        var start = new ProcessStartInfo(executable)
        {
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardInput = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            StandardInputEncoding = new UTF8Encoding(false),
            StandardOutputEncoding = Encoding.UTF8,
            StandardErrorEncoding = Encoding.UTF8,
            WorkingDirectory = projectPath
        };
        start.ArgumentList.Add("app-server");
        start.ArgumentList.Add("-c");
        start.ArgumentList.Add("model_provider=\"openai\"");
        // Configure only this child process, never the user's global Codex config.
        start.ArgumentList.Add("-c");
        start.ArgumentList.Add("mcp_servers.pixeltool.enabled=" + (connectLocalMcp ? "true" : "false"));
        start.ArgumentList.Add("-c");
        start.ArgumentList.Add($"mcp_servers.pixeltool.url=\"http://127.0.0.1:{PixelToolMcpServer.Port}/mcp\"");
        if (connectLocalMcp)
        {
            string tokenPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "PixelTool", "Mcp", "token.txt");
            string token = File.ReadAllText(tokenPath);
            if (token.Length != 64 || !token.All(Uri.IsHexDigit)) throw new InvalidDataException("PixelTool MCP token is invalid.");
            start.Environment["PIXELTOOL_MCP_TOKEN"] = token;
            start.ArgumentList.Add("-c");
            start.ArgumentList.Add("mcp_servers.pixeltool.bearer_token_env_var=\"PIXELTOOL_MCP_TOKEN\"");
            start.ArgumentList.Add("-c");
            start.ArgumentList.Add("mcp_servers.pixeltool.http_headers={}");
        }
        // Do not silently use an API key inherited from the editor's environment.
        start.Environment.Remove("OPENAI_API_KEY");
        start.Environment.Remove("CODEX_API_KEY");
        StartTransport(start);
        try
        {
            await RequestAsync("initialize", new { clientInfo = new { name = "pixeltool", title = "PixelTool AI Chat", version = "0.1.0" } }).ConfigureAwait(false);
            await WriteAsync(new { method = "initialized", @params = new { } }).ConfigureAwait(false);
        }
        catch
        {
            await DisposeAsync().ConfigureAwait(false);
            throw;
        }
    }

    private void StartTransport(ProcessStartInfo start)
    {
        if (_process != null) throw new InvalidOperationException("Codex is already connected.");
        start.StandardOutputEncoding = start.StandardErrorEncoding = Encoding.UTF8;
        _process = Process.Start(start) ?? throw new IOException("Codex CLI를 실행하지 못했습니다.");
        // Start stderr first so even an immediate startup failure is included in the report.
        _stderr = DrainErrorsAsync(_process.StandardError);
        _reader = ReadAsync(_process);
    }

    public async Task<JsonElement> RequestAsync(string method, object parameters)
    {
        long id = Interlocked.Increment(ref _nextId);
        var completion = new TaskCompletionSource<JsonElement>(TaskCreationOptions.RunContinuationsAsynchronously);
        _pending[id] = completion;
        try
        {
            await WriteAsync(new { id, method, @params = parameters }).ConfigureAwait(false);
            return await completion.Task.WaitAsync(TimeSpan.FromSeconds(90)).ConfigureAwait(false);
        }
        finally
        {
            _pending.TryRemove(id, out _);
        }
    }

    private async Task WriteAsync(object message)
    {
        await _writeLock.WaitAsync().ConfigureAwait(false);
        try
        {
            var process = _process ?? throw new IOException("Codex is not connected.");
            try
            {
                await process.StandardInput.WriteLineAsync(JsonSerializer.Serialize(message)).ConfigureAwait(false);
                await process.StandardInput.FlushAsync().ConfigureAwait(false);
            }
            catch (Exception exception) when (!_closing && exception is IOException or ObjectDisposedException)
            {
                lock (_diagnosticLock) _transportFailure ??= "요청 전송: " + exception.GetType().Name + ": " + SanitizeDiagnostic(exception.Message);
                throw new IOException(await DisconnectReportAsync(process).ConfigureAwait(false));
            }
        }
        finally { _writeLock.Release(); }
    }

    private async Task ReadAsync(Process process)
    {
        string stage = "CLI 응답 읽기";
        try
        {
            while (await process.StandardOutput.ReadLineAsync(_transportStop.Token).ConfigureAwait(false) is { } line)
            {
                stage = "JSON 응답 해석";
                using var document = JsonDocument.Parse(line);
                var root = document.RootElement;
                if (root.TryGetProperty("method", out var method))
                {
                    var parameters = root.TryGetProperty("params", out var p) ? p.Clone() : default;
                    if (root.TryGetProperty("id", out var requestId))
                        _ = RespondAsync(requestId.Clone(), method.GetString()!, parameters);
                    else
                    {
                        stage = "CLI 알림 처리: " + method.GetString();
                        Notification?.Invoke(method.GetString()!, parameters);
                    }
                }
                else if (root.TryGetProperty("id", out var id) && id.ValueKind == JsonValueKind.Number && id.TryGetInt64(out long number) && _pending.TryGetValue(number, out var completion))
                {
                    if (root.TryGetProperty("error", out var error))
                        completion.TrySetException(new InvalidOperationException(error.GetProperty("message").GetString()));
                    else if (root.TryGetProperty("result", out var result))
                        completion.TrySetResult(result.Clone());
                }
                stage = "CLI 응답 읽기";
            }
        }
        catch (Exception exception)
        {
            if (!_closing)
            {
                // JsonException.Message can contain a fragment of the stdout payload.
                string message = exception is JsonException json ? $"JSON 형식 오류 (줄 {json.LineNumber}, 바이트 {json.BytePositionInLine})." : exception.Message;
                string safe = SanitizeDiagnostic(stage + ": " + exception.GetType().Name + ": " + message);
                lock (_diagnosticLock) _transportFailure = safe.Length > 1500 ? safe[..1500] + "…" : safe;
            }
        }
        finally
        {
            string details = _closing ? "Codex connection closed." : await DisconnectReportAsync(process).ConfigureAwait(false);
            foreach (var completion in _pending.Values) completion.TrySetException(new IOException(details));
            if (!_closing) Disconnected?.Invoke(details);
        }
    }

    private async Task RespondAsync(JsonElement id, string method, JsonElement parameters)
    {
        try
        {
            object result;
            if (method == "item/tool/call" && CallTool != null)
            {
                result = await CallTool(parameters).ConfigureAwait(false);
            }
            else if (method is "item/commandExecution/requestApproval" or "item/fileChange/requestApproval")
            {
                bool accepted = !_closing && Approve != null && await Approve(method, parameters).ConfigureAwait(false);
                result = new { decision = accepted ? "accept" : "decline" };
            }
            else if (method == "item/permissions/requestApproval")
            {
                // Never grant broad extra filesystem/network permissions automatically.
                result = new { permissions = new { }, scope = "turn" };
            }
            else
            {
                await WriteAsync(new { id, error = new { code = -32601, message = "This PixelTool client does not support this interactive request. Ask the user in chat instead." } }).ConfigureAwait(false);
                return;
            }
            await WriteAsync(new { id, result }).ConfigureAwait(false);
        }
        catch (Exception) when (_closing) { }
        catch (Exception)
        {
            foreach (var completion in _pending.Values) completion.TrySetException(new IOException("Could not answer a Codex approval request."));
        }
    }

    private async Task DrainErrorsAsync(StreamReader reader)
    {
        try
        {
            while (await reader.ReadLineAsync(_transportStop.Token).ConfigureAwait(false) is { } line)
            {
                string safe = SanitizeDiagnostic(line);
                if (string.IsNullOrWhiteSpace(safe)) continue;
                lock (_diagnosticLock)
                {
                    _errorTail.Enqueue(safe.Length > 1000 ? safe[..1000] + "…" : safe);
                    while (_errorTail.Count > 12) _errorTail.Dequeue();
                }
            }
        }
        catch (Exception exception) when (exception is IOException or ObjectDisposedException or OperationCanceledException) { }
    }

    private Task<string> DisconnectReportAsync(Process process)
    {
        lock (_diagnosticLock) return _disconnectReport ??= BuildDisconnectReportAsync(process);
    }

    private async Task<string> BuildDisconnectReportAsync(Process process)
    {
        int? exitCode = null;
        try
        {
            if (!process.HasExited)
                await process.WaitForExitAsync().WaitAsync(TimeSpan.FromSeconds(1)).ConfigureAwait(false);
            exitCode = process.ExitCode;
            if (_stderr != null) await _stderr.WaitAsync(TimeSpan.FromMilliseconds(500)).ConfigureAwait(false);
        }
        catch (Exception exception) when (exception is TimeoutException or InvalidOperationException or ObjectDisposedException or System.ComponentModel.Win32Exception) { }
        string reason, stderr;
        lock (_diagnosticLock)
        {
            reason = _transportFailure ?? "CLI의 표준 출력 연결이 종료되었습니다.";
            stderr = string.Join("\n", _errorTail);
        }
        if (stderr.Length > 4000) stderr = stderr[..4000] + "\n…(오류 출력 일부 생략)";
        string code = exitCode is int value ? $"{value} (0x{unchecked((uint)value):X8})" : "확인 불가 · 프로세스가 실행 중이거나 종료 상태를 읽을 수 없습니다.";
        return $"CLI 종료 코드: {code}\n통신 오류: {reason}\nCLI 오류 출력(stderr):\n{(stderr.Length == 0 ? "(출력 없음)" : stderr)}";
    }

    // Sanitize before retaining any stderr. Never retain stdout payloads or authentication files.
    internal static string SanitizeDiagnostic(string text)
    {
        text = Regex.Replace(text, @"\x1B\[[0-?]*[ -/]*[@-~]", "");
        text = Regex.Replace(text, @"(?i)\bBearer\s+[^\s""'<>]+", "Bearer [REDACTED]");
        text = Regex.Replace(text, @"(?im)(authorization|proxy-authorization|cookie|set-cookie)\s*[:=][^\r\n]*", "$1: [REDACTED]");
        text = Regex.Replace(text, @"(?i)([""']?(?:[a-z0-9_]*api[_-]?key|[a-z0-9_]*token|client[_-]?secret|password)[""']?\s*[:=]\s*)(?:""[^""]*""|'[^']*'|[^\s,;}\]]+)", "$1[REDACTED]");
        text = Regex.Replace(text, @"\bsk-[A-Za-z0-9_-]+|\beyJ[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+|\b[a-fA-F0-9]{64}\b", "[REDACTED]");
        return Regex.Replace(text, @"(?i)https?://[^\s""'<>]+", "[URL REDACTED]");
    }

    public async ValueTask DisposeAsync()
    {
        _closing = true;
        var process = Interlocked.Exchange(ref _process, null);
        if (process == null) return;
        try
        {
            _transportStop.Cancel();
            foreach (var completion in _pending.Values) completion.TrySetException(new IOException("Codex connection closed."));
            // Only this client's child process tree is owned by the editor. Do not
            // flush stdin or wait for EOF: a child may still hold a pipe open.
            try { if (!process.HasExited) process.Kill(entireProcessTree: true); }
            catch (InvalidOperationException) { } // Already exited.
            try { await process.WaitForExitAsync().WaitAsync(TimeSpan.FromSeconds(2)).ConfigureAwait(false); }
            catch (TimeoutException) { Debug.WriteLine("Codex process shutdown timed out."); }
            try
            {
                await Task.WhenAll(_reader ?? Task.CompletedTask, _stderr ?? Task.CompletedTask)
                    .WaitAsync(TimeSpan.FromSeconds(2)).ConfigureAwait(false);
            }
            catch (TimeoutException) { Debug.WriteLine("Codex pipe shutdown timed out."); }
        }
        finally
        {
            process.Dispose();
        }
    }
}
