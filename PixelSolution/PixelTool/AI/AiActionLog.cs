using System.Text.Json;
using System.Text.RegularExpressions;

namespace PixelTool;

// Operation metadata only: never dump tool arguments, file contents or credentials.
internal static class AiActionLog
{
    private static readonly AsyncLocal<string?> RequestId = new();
    private static string? _activeRequestId;
    private static long _eventId;
    internal static string? CurrentRequestId => RequestId.Value ?? Volatile.Read(ref _activeRequestId);

    internal static string BeginRequest(string text)
    {
        string id = Guid.NewGuid().ToString("N");
        ConsoleWindow.BeginAiLogEntry();
        Volatile.Write(ref _activeRequestId, id);
        using (Enter(id)) ConsoleWindow.LogAi("사용자 요청: " + text);
        return id;
    }

    internal static void EndRequest(string? id)
    {
        if (id != null) Interlocked.CompareExchange(ref _activeRequestId, null, id);
    }

    internal static IDisposable Enter(string? id)
    {
        var scope = new RequestScope(RequestId.Value);
        RequestId.Value = id;
        return scope;
    }

    internal static string Prefix(string message) =>
        $"요청={CurrentRequestId ?? "미연결"} | 이벤트={Interlocked.Increment(ref _eventId)} | {message}";

    private sealed class RequestScope(string? previous) : IDisposable
    {
        private bool _disposed;
        public void Dispose()
        {
            if (_disposed) return;
            RequestId.Value = previous;
            _disposed = true;
        }
    }

    internal static string Clean(string? text)
    {
        string clean = CodexChatClient.SanitizeDiagnostic(text ?? "");
        clean = Regex.Replace(clean, @"[\r\n\t]+", " ");
        return clean.Length > 4000 ? clean[..4000] + " [truncated]" : clean;
    }

    internal static string Target(string operation, JsonElement args)
    {
        if (args.ValueKind != JsonValueKind.Object) return "";
        var parts = new List<string>();
        foreach (string key in new[] { "path", "source", "destination", "sourcePath", "destinationPath", "document", "mode", "key", "startLine", "lineCount" })
            if (args.TryGetProperty(key, out var value) && value.ValueKind is JsonValueKind.String or JsonValueKind.Number)
                parts.Add(key + "=" + value.ToString());
        if (operation == "read_wiki" && args.TryGetProperty("requests", out var requests) && requests.ValueKind == JsonValueKind.Array)
        {
            int count = 0;
            foreach (var request in requests.EnumerateArray())
            {
                if (++count > EngineGuide.MaxBatchPages) { parts.Add("additionalPages=omitted"); break; }
                if (request.ValueKind != JsonValueKind.Object) continue;
                string document = request.TryGetProperty("document", out var id) && id.ValueKind == JsonValueKind.String
                    ? Clean(id.GetString()) : "?";
                if (document.Length > 160) document = document[..160] + "…";
                string offset = request.TryGetProperty("offset", out var start) && start.ValueKind == JsonValueKind.Number ? start.ToString() : "?";
                string length = request.TryGetProperty("length", out var size) && size.ValueKind == JsonValueKind.Number ? size.ToString() : "?";
                parts.Add($"page{count}={document} offset={offset} length={length}");
            }
        }
        return string.Join(", ", parts);
    }

    internal static string FailureReason(string result)
    {
        try { using var doc = JsonDocument.Parse(result); return FailureReason(doc.RootElement); }
        catch (JsonException) { return ""; }
    }

    private static string FailureReason(JsonElement result)
    {
        if (result.ValueKind != JsonValueKind.Object) return "";
        bool failed = new[] { "success", "saved", "ok" }.Any(key => result.TryGetProperty(key, out var value) && value.ValueKind == JsonValueKind.False);
        if (result.TryGetProperty("status", out var status) && status.ValueKind == JsonValueKind.String)
            failed |= status.GetString() is "failed" or "error" or "cancelled" or "canceled" or "needs_context";
        if (failed)
        {
            foreach (string key in new[] { "message", "error", "reason", "summary", "Summary" })
                if (result.TryGetProperty(key, out var value) && value.ValueKind == JsonValueKind.String && !string.IsNullOrWhiteSpace(value.GetString()))
                    return value.GetString()!;
            if (result.TryGetProperty("contentItems", out var items) && items.ValueKind == JsonValueKind.Array)
                foreach (var item in items.EnumerateArray())
                    if (item.TryGetProperty("text", out var text)) return text.GetString() ?? "실패 이유가 반환되지 않았습니다.";
            return "실패 이유가 반환되지 않았습니다.";
        }
        if (result.TryGetProperty("data", out var data)) return FailureReason(data);
        return "";
    }

    private static long _callId;
    private static readonly object CallsLock = new();
    private static readonly Dictionary<(string?, string, string), (long Count, long Last)> Calls = new();
    private static readonly Queue<(string?, string, string)> CallOrder = new();

    private static (long Id, long Attempt, long Previous) NextCall(string operation, string target)
    {
        lock (CallsLock)
        {
            long id = ++_callId;
            if (CurrentRequestId == null) return (id, 1, 0);
            var key = (CurrentRequestId, Clean(operation), Clean(target));
            bool found = Calls.TryGetValue(key, out var prior);
            if (!found)
            {
                if (Calls.Count >= 2000) Calls.Remove(CallOrder.Dequeue());
                CallOrder.Enqueue(key);
            }
            Calls[key] = (prior.Count + 1, id);
            return (id, prior.Count + 1, prior.Last);
        }
    }

    private static string FailureCode(string result)
    {
        try { using var doc = JsonDocument.Parse(result); return FindCode(doc.RootElement) ?? "TOOL_REPORTED_FAILURE"; }
        catch (JsonException) { return "TOOL_REPORTED_FAILURE"; }
    }

    private static string? FindCode(JsonElement value)
    {
        if (value.ValueKind != JsonValueKind.Object) return null;
        foreach (string key in new[] { "code", "error" })
            if (value.TryGetProperty(key, out var code) && code.ValueKind == JsonValueKind.String && Regex.IsMatch(code.GetString() ?? "", @"^[A-Za-z][A-Za-z0-9_.-]{0,63}$"))
                return code.GetString();
        if (value.TryGetProperty("data", out var data) && FindCode(data) is { } nested) return nested;
        if (value.TryGetProperty("contentItems", out var items) && items.ValueKind == JsonValueKind.Array)
            foreach (var item in items.EnumerateArray())
                if (item.ValueKind == JsonValueKind.Object && item.TryGetProperty("text", out var text) && text.ValueKind == JsonValueKind.String)
                    try { using var doc = JsonDocument.Parse(text.GetString()!); if (FindCode(doc.RootElement) is { } code) return code; }
                    catch (JsonException) { }
        return null;
    }

    private static string CallDetails((long Id, long Attempt, long Previous) call) =>
        $"호출ID={call.Id} | 같은대상호출차수={call.Attempt} | 이전호출ID={(call.Previous == 0 ? "없음" : call.Previous.ToString())}";

    internal static bool Record(string operation, string target, string result, bool success = true, string changeState = "확인불가", string stage = "도구결과")
    {
        var call = NextCall(operation, target);
        string reason = FailureReason(result);
        success &= reason.Length == 0;
        if (!success && reason.Length == 0) reason = result;
        ConsoleWindow.LogAi(operation + (target.Length == 0 ? "" : " | " + target) +
            (success ? $" | 완료 | 호출ID={call.Id}" : $" | 실패: | {CallDetails(call)} | 코드={FailureCode(result)} | 단계={stage} | 변경여부={changeState} | 원인: {reason}"));
        return success;
    }

    internal static void Failed(string operation, string target, Exception ex)
    {
        var call = NextCall(operation, target);
        string changeState = ex.Data["AiChangeState"] as string ?? "확인불가";
        ConsoleWindow.LogAi(operation + (target.Length == 0 ? "" : " | " + target) +
            (ex is OperationCanceledException ? " | 취소:" : " | 실패:") +
            $" | {CallDetails(call)} | 코드={ex.GetType().Name} | 단계=예외 | HResult=0x{ex.HResult:X8} | 변경여부={changeState} | 원인: {ex.Message}");
    }
}
