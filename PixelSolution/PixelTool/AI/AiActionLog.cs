using System.Text.Json;
using System.Text.RegularExpressions;

namespace PixelTool;

// Operation metadata only: never dump tool arguments, file contents or credentials.
internal static class AiActionLog
{
    private static readonly AsyncLocal<string?> RequestId = new();
    private static string? _activeRequestId;
    private static long _eventId;
    private static readonly Lazy<Dictionary<string, string[]>> RequiredToolArguments = new(() =>
        ChatGptProjectTools.Definitions.Select(definition => JsonSerializer.SerializeToElement(definition))
            .ToDictionary(definition => definition.GetProperty("name").GetString()!, definition =>
                definition.GetProperty("parameters").GetProperty("required").EnumerateArray()
                    .Select(item => item.GetString()!).Where(name => name != "allowRootPlacement").ToArray(), StringComparer.Ordinal));
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
        if (args.ValueKind != JsonValueKind.Object) return "인자형식=" + args.ValueKind;
        var parts = new List<string>();
        foreach (string key in new[] { "path", "source", "destination", "sourcePath", "destinationPath", "document", "mode", "key", "startLine", "lineCount" })
            if (args.TryGetProperty(key, out var value) && value.ValueKind is JsonValueKind.String or JsonValueKind.Number)
                parts.Add(key + "=" + Short(value.ToString()));
        if (operation == "patch_project_file" && !args.TryGetProperty("path", out _))
            parts.Add("path=<missing>");
        if (operation == "search_project_work_items" && args.TryGetProperty("query", out var query) && query.ValueKind == JsonValueKind.String)
            parts.Add("query=" + Short(query.GetString()));
        if (operation == "create_project_work_item")
        {
            if (args.TryGetProperty("title", out var title) && title.ValueKind == JsonValueKind.String)
                parts.Add("title=" + Short(title.GetString()));
            if (args.TryGetProperty("checks", out var checks) && checks.ValueKind == JsonValueKind.Array)
                parts.Add("checks=" + checks.GetArrayLength());
        }
        if (operation == "read_wiki" && args.TryGetProperty("requests", out var requests))
            AppendRanges(parts, requests, "document", "page", EngineGuide.MaxBatchPages);
        if (operation == "read_project_file_lines" && args.TryGetProperty("requests", out var ranges))
            AppendRanges(parts, ranges, "path", "file", 4);
        if (operation == "read_project_context")
        {
            if (args.TryGetProperty("guides", out var guides)) AppendRanges(parts, guides, "document", "page", EngineGuide.MaxBatchPages);
            if (args.TryGetProperty("files", out var files)) AppendRanges(parts, files, "path", "file", 4);
        }
        if (RequiredToolArguments.Value.TryGetValue(operation, out var required))
        {
            string[] present = required.Where(key => args.TryGetProperty(key, out _)).ToArray();
            string[] missing = required.Except(present, StringComparer.Ordinal).ToArray();
            parts.Add("인자=" + (present.Length == 0 ? "없음" : string.Join(",", present.Select(key =>
            {
                var value = args.GetProperty(key);
                return key + ":" + value.ValueKind + (value.ValueKind == JsonValueKind.String
                    ? "(" + (value.GetString()?.Length ?? 0) + "자)"
                    : value.ValueKind == JsonValueKind.Array ? "(" + value.GetArrayLength() + "개)" : "");
            }))));
            if (missing.Length > 0) parts.Add("필수누락=" + string.Join(",", missing));
            int extra = args.EnumerateObject().Count(property => !required.Contains(property.Name, StringComparer.Ordinal));
            if (extra > 0) parts.Add("미등록인자=" + extra + "개");
        }
        return string.Join(", ", parts);
    }

    private static string Short(string? value)
    {
        string clean = Clean(value);
        return clean.Length > 160 ? clean[..160] + "…" : clean;
    }

    internal static string SearchTarget(string path, string query) =>
        "path=" + Short(path) + ", query=" + Short(query);

    internal static string WorkItemUpdateTarget(string document, string changeType, string[]? changedFiles) =>
        "document=" + Short(document) + ", type=" + Short(changeType) +
        ", files=" + string.Join(";", (changedFiles ?? []).Take(20).Select(Short));

    private static void AppendRanges(List<string> parts, JsonElement ranges, string nameKey, string label, int max)
    {
        if (ranges.ValueKind != JsonValueKind.Array) return;
        int count = 0;
        foreach (var range in ranges.EnumerateArray())
        {
            if (++count > max) { parts.Add("additionalRanges=omitted"); break; }
            if (range.ValueKind != JsonValueKind.Object) continue;
            string name = range.TryGetProperty(nameKey, out var value) && value.ValueKind == JsonValueKind.String
                ? Short(value.GetString()) : "?";
            var details = new List<string>();
            foreach (string key in nameKey == "document" ? new[] { "offset", "length" } : new[] { "startLine", "lineCount", "startColumn" })
            {
                if (range.TryGetProperty(key, out var number) && number.ValueKind == JsonValueKind.Number)
                    details.Add(key + "=" + number);
            }
            parts.Add($"{label}{count}={name}" + (details.Count == 0 ? "" : " " + string.Join(" ", details)));
        }
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

    internal static string FailureStage(string result) => FailureCode(result) == "missing_argument" ? "인자검증" : "도구결과";

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
        string summary = success ? ResultSummary(operation, result) : "";
        ConsoleWindow.LogAi(operation + (target.Length == 0 ? "" : " | " + target) +
            (summary.Length == 0 ? "" : " | " + summary) +
            (success ? $" | 완료 | 호출ID={call.Id}" : $" | 실패: | {CallDetails(call)} | 코드={FailureCode(result)} | 단계={stage} | 변경여부={changeState} | 원인: {reason}"));
        return success;
    }

    private static string ResultSummary(string operation, string result)
    {
        if (operation is not ("AI 도구 search_project_work_items" or "AI 도구 create_project_work_item" or
            "MCP Asset_File_Search" or "MCP Asset_FileContent_Search" or "MCP Editor_WorkItem_Update")) return "";
        try
        {
            using var doc = JsonDocument.Parse(result);
            var root = doc.RootElement;
            if (operation == "AI 도구 search_project_work_items" && root.TryGetProperty("matches", out var matches) && matches.ValueKind == JsonValueKind.Array)
                return "결과=" + matches.GetArrayLength();
            if (operation == "AI 도구 create_project_work_item" && root.TryGetProperty("path", out var path) && path.ValueKind == JsonValueKind.String)
                return "생성=" + Short(path.GetString());
            if (operation == "MCP Editor_WorkItem_Update" && root.TryGetProperty("data", out var update) && update.ValueKind == JsonValueKind.Object &&
                update.TryGetProperty("updateId", out var updateId) && updateId.ValueKind == JsonValueKind.String)
                return "updateId=" + Short(updateId.GetString());
            if (operation.StartsWith("MCP Asset_", StringComparison.Ordinal) && root.TryGetProperty("data", out var data) && data.ValueKind == JsonValueKind.Object)
                foreach (string key in new[] { "entries", "matches" })
                    if (data.TryGetProperty(key, out var items) && items.ValueKind == JsonValueKind.Array)
                        return "결과=" + items.GetArrayLength();
        }
        catch (JsonException) { }
        return "";
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
