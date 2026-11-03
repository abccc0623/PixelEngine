using System.Collections.Concurrent;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.Json;

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
    public event Action<string, JsonElement>? Notification;
    public event Action? Disconnected;
    public Func<string, JsonElement, Task<bool>>? Approve { get; set; }

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
        _process = Process.Start(start) ?? throw new IOException("Could not start Codex.");
        _reader = ReadAsync(_process.StandardOutput);
        _stderr = DrainErrorsAsync(_process.StandardError);
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
            await process.StandardInput.WriteLineAsync(JsonSerializer.Serialize(message)).ConfigureAwait(false);
            await process.StandardInput.FlushAsync().ConfigureAwait(false);
        }
        finally { _writeLock.Release(); }
    }

    private async Task ReadAsync(StreamReader reader)
    {
        try
        {
            while (await reader.ReadLineAsync(_transportStop.Token).ConfigureAwait(false) is { } line)
            {
                using var document = JsonDocument.Parse(line);
                var root = document.RootElement;
                if (root.TryGetProperty("method", out var method))
                {
                    var parameters = root.TryGetProperty("params", out var p) ? p.Clone() : default;
                    if (root.TryGetProperty("id", out var requestId))
                        _ = RespondAsync(requestId.Clone(), method.GetString()!, parameters);
                    else
                        Notification?.Invoke(method.GetString()!, parameters);
                }
                else if (root.TryGetProperty("id", out var id) && id.ValueKind == JsonValueKind.Number && id.TryGetInt64(out long number) && _pending.TryGetValue(number, out var completion))
                {
                    if (root.TryGetProperty("error", out var error))
                        completion.TrySetException(new InvalidOperationException(error.GetProperty("message").GetString()));
                    else if (root.TryGetProperty("result", out var result))
                        completion.TrySetResult(result.Clone());
                }
            }
        }
        catch (Exception exception) when (exception is IOException or ObjectDisposedException or JsonException or OperationCanceledException)
        {
            // Transport errors are surfaced below, without exposing auth or stderr content.
        }
        finally
        {
            foreach (var completion in _pending.Values) completion.TrySetException(new IOException("Codex connection closed."));
            if (!_closing) Disconnected?.Invoke();
        }
    }

    private async Task RespondAsync(JsonElement id, string method, JsonElement parameters)
    {
        try
        {
            object result;
            if (method is "item/commandExecution/requestApproval" or "item/fileChange/requestApproval")
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
            while (await reader.ReadLineAsync(_transportStop.Token).ConfigureAwait(false) != null) { }
        }
        catch (Exception exception) when (exception is IOException or ObjectDisposedException or OperationCanceledException) { }
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
