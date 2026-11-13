using System.Globalization;
using System.Text.Json;

namespace PixelTool;

internal sealed record CodexTokenCounts(long TotalTokens, long InputTokens, long CachedInputTokens,
    long CacheWriteInputTokens, long OutputTokens, long ReasoningOutputTokens);

internal sealed record CodexThreadUsage(string ThreadId, string Label, CodexTokenCounts? Total,
    CodexTokenCounts? Last, CodexTokenCounts? Work, long? ModelContextWindow)
{
    public long? WorkTokens => Work?.TotalTokens;

    // The last model call is context occupancy, not the sum of the conversation's calls.
    public double? ContextPercent => Last != null && ModelContextWindow is > 0
        ? 100d * Last.TotalTokens / ModelContextWindow.Value : null;
}

internal sealed record CodexUsageSnapshot(string Text, CodexTokenCounts? Work, bool HasUsage,
    CodexThreadUsage? Root)
{
    public long? TotalTokens => Work?.TotalTokens;
}

// App-server sends cumulative snapshots. Never add snapshots or add cache/reasoning
// counters to totalTokens: those counters are already included by the provider.
internal sealed class CodexUsageTracker
{
    private static readonly CodexTokenCounts Zero = new(0, 0, 0, 0, 0, 0);

    private sealed class ThreadState
    {
        public string Label = "";
        public CodexTokenCounts? Total;
        public CodexTokenCounts? Last;
        public long? Window;
        public CodexTokenCounts? Baseline;
        public bool Observed;
        public bool Invalid;
    }

    private readonly object _gate = new();
    private readonly Dictionary<string, ThreadState> _threads = new(StringComparer.Ordinal);
    private string? _root;

    public void BeginWork(string rootThreadId, bool newThread)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(rootThreadId);
        lock (_gate)
        {
            _root = rootThreadId;
            if (newThread) _threads.Remove(rootThreadId);
            ThreadState state = Get(rootThreadId);
            state.Label = "메인";
            state.Baseline = newThread ? Zero : state.Total;
            state.Observed = false;
            state.Invalid = false;
        }
    }

    // Accept either notification params or a full JSON-RPC notification envelope.
    public bool Observe(JsonElement notification)
    {
        if (notification.ValueKind != JsonValueKind.Object) return false;
        if (notification.TryGetProperty("method", out var method))
        {
            if (method.ValueKind != JsonValueKind.String || method.GetString() != "thread/tokenUsage/updated") return false;
            if (!notification.TryGetProperty("params", out notification) || notification.ValueKind != JsonValueKind.Object) return false;
        }
        if (!notification.TryGetProperty("threadId", out var id) || id.ValueKind != JsonValueKind.String
            || string.IsNullOrWhiteSpace(id.GetString())) return false;
        lock (_gate)
        {
            if (id.GetString() != _root) return false;
            ThreadState state = Get(id.GetString()!);
            if (!notification.TryGetProperty("tokenUsage", out var usage) || usage.ValueKind != JsonValueKind.Object
                || !usage.TryGetProperty("total", out var totalJson) || !TryCounts(totalJson, out var total))
            {
                state.Invalid = true;
                // A malformed latest snapshot cannot be the next request's baseline.
                state.Total = state.Last = null;
                state.Window = null;
                return false;
            }
            if ((state.Total != null && Decreased(total!, state.Total))
                || (state.Baseline != null && Difference(total!, state.Baseline) == null)) state.Invalid = true;
            state.Total = total;
            state.Observed = true;
            state.Last = usage.TryGetProperty("last", out var last) && TryCounts(last, out var counts) ? counts : null;
            state.Window = usage.TryGetProperty("modelContextWindow", out var window)
                && window.ValueKind == JsonValueKind.Number && window.TryGetInt64(out long size) && size > 0 ? size : null;
            return true;
        }
    }

    public CodexUsageSnapshot Snapshot()
    {
        lock (_gate)
        {
            CodexThreadUsage? root = _root == null ? null : Copy(_root);
            CodexTokenCounts? sum = root?.Work;
            bool hasUsage = root?.Total != null;
            string text = "이번 요청 " + FormatCounts(sum);
            if (root != null)
            {
                text += "\n메인 대화 누적 " + FormatCounts(root.Total);
                text += root.ContextPercent is double percent
                    ? " · 컨텍스트 " + percent.ToString("0.#", CultureInfo.InvariantCulture) + "%"
                    : " · 컨텍스트 미측정";
            }
            return new(text, sum, hasUsage, root);
        }
    }

    private static string FormatCounts(CodexTokenCounts? counts) => counts == null ? "미측정/불완전"
        : "합계 " + Number(counts.TotalTokens) + " · 입력 " + Number(counts.InputTokens)
            + " (캐시 " + Number(counts.CachedInputTokens) + " 포함) · 출력 " + Number(counts.OutputTokens);

    private static string Number(long? value) => value?.ToString("N0", CultureInfo.InvariantCulture) ?? "미측정/불완전";

    private ThreadState Get(string id)
    {
        if (!_threads.TryGetValue(id, out var state)) _threads[id] = state = new ThreadState();
        return state;
    }

    private CodexThreadUsage Copy(string id)
    {
        ThreadState state = Get(id);
        CodexTokenCounts? work = !state.Invalid && state.Observed && state.Baseline != null && state.Total != null
            ? Difference(state.Total, state.Baseline) : null;
        return new(id, state.Label, state.Total, state.Last, work, state.Window);
    }

    private static CodexTokenCounts? Difference(CodexTokenCounts total, CodexTokenCounts baseline)
    {
        if (Decreased(total, baseline)) return null;
        var delta = new CodexTokenCounts(total.TotalTokens - baseline.TotalTokens,
            total.InputTokens - baseline.InputTokens, total.CachedInputTokens - baseline.CachedInputTokens,
            total.CacheWriteInputTokens - baseline.CacheWriteInputTokens, total.OutputTokens - baseline.OutputTokens,
            total.ReasoningOutputTokens - baseline.ReasoningOutputTokens);
        return Consistent(delta) ? delta : null;
    }

    private static bool Decreased(CodexTokenCounts a, CodexTokenCounts b) =>
        a.TotalTokens < b.TotalTokens || a.InputTokens < b.InputTokens || a.CachedInputTokens < b.CachedInputTokens
        || a.CacheWriteInputTokens < b.CacheWriteInputTokens || a.OutputTokens < b.OutputTokens
        || a.ReasoningOutputTokens < b.ReasoningOutputTokens;

    private static bool Consistent(CodexTokenCounts counts) => counts.InputTokens <= counts.TotalTokens
        && counts.TotalTokens - counts.InputTokens == counts.OutputTokens
        && counts.CachedInputTokens <= counts.InputTokens && counts.CacheWriteInputTokens <= counts.InputTokens
        && counts.ReasoningOutputTokens <= counts.OutputTokens;

    private static bool TryCounts(JsonElement json, out CodexTokenCounts? counts)
    {
        counts = null;
        if (json.ValueKind != JsonValueKind.Object) return false;
        string[] names = ["totalTokens", "inputTokens", "cachedInputTokens", "cacheWriteInputTokens", "outputTokens", "reasoningOutputTokens"];
        long[] values = new long[names.Length];
        for (int i = 0; i < names.Length; i++)
            if (!json.TryGetProperty(names[i], out var value) || value.ValueKind != JsonValueKind.Number
                || !value.TryGetInt64(out values[i]) || values[i] < 0) return false;
        var parsed = new CodexTokenCounts(values[0], values[1], values[2], values[3], values[4], values[5]);
        if (!Consistent(parsed)) return false;
        counts = parsed;
        return true;
    }
}
