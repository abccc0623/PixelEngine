using System.Globalization;
using System.Text.Json;

namespace PixelTool;

internal sealed record CodexTokenCounts(long TotalTokens, long InputTokens, long CachedInputTokens,
    long CacheWriteInputTokens, long OutputTokens, long ReasoningOutputTokens);

internal sealed record CodexThreadUsage(string ThreadId, string Label, CodexTokenCounts? Total,
    CodexTokenCounts? Last, long? WorkTokens, long? ModelContextWindow)
{
    // The last model call is context occupancy, not the sum of the conversation's calls.
    public double? ContextPercent => Last != null && ModelContextWindow is > 0
        ? 100d * Last.TotalTokens / ModelContextWindow.Value : null;
}

internal sealed record CodexUsageSnapshot(string Text, long? TotalTokens, bool HasUsage,
    CodexThreadUsage? Root, IReadOnlyList<CodexThreadUsage> Workers);

// App-server sends cumulative snapshots. Never add snapshots or add cache/reasoning
// counters to totalTokens: those counters are already included by the provider.
internal sealed class CodexUsageTracker
{
    private sealed class ThreadState
    {
        public string Label = "";
        public CodexTokenCounts? Total;
        public CodexTokenCounts? Last;
        public long? Window;
        public long? Baseline;
        public bool Observed;
        public bool Invalid;
    }

    private readonly object _gate = new();
    private readonly Dictionary<string, ThreadState> _threads = new(StringComparer.Ordinal);
    private readonly List<string> _workers = new();
    private string? _root;

    public void BeginWork(string rootThreadId, bool newThread)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(rootThreadId);
        lock (_gate)
        {
            _root = rootThreadId;
            _workers.Clear();
            if (newThread) _threads.Remove(rootThreadId);
            ThreadState state = Get(rootThreadId);
            state.Label = "메인";
            state.Baseline = newThread ? 0 : state.Total?.TotalTokens;
            state.Observed = false;
            state.Invalid = false;
        }
    }

    // Call only for newly created worker threads. Repeated registration is idempotent.
    public void RegisterWorker(string threadId, string label)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(threadId);
        lock (_gate)
        {
            if (threadId == _root) return;
            if (!_workers.Contains(threadId))
            {
                _workers.Add(threadId);
                Get(threadId).Baseline = 0;
            }
            Get(threadId).Label = label;
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
            || string.IsNullOrWhiteSpace(id.GetString()) || !notification.TryGetProperty("tokenUsage", out var usage)
            || usage.ValueKind != JsonValueKind.Object) return false;
        lock (_gate)
        {
            ThreadState state = Get(id.GetString()!);
            if (!usage.TryGetProperty("total", out var totalJson) || !TryCounts(totalJson, out var total))
            {
                state.Invalid = true;
                return false;
            }
            if (state.Total != null && Decreased(total!, state.Total)) state.Invalid = true;
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
            var workers = _workers.Select(Copy).ToArray();
            long? sum = root?.WorkTokens;
            foreach (var worker in workers)
            {
                if (!sum.HasValue || !worker.WorkTokens.HasValue || long.MaxValue - sum.Value < worker.WorkTokens.Value)
                    sum = null;
                else sum += worker.WorkTokens.Value;
            }
            bool hasUsage = root?.Total != null || workers.Any(w => w.Total != null);
            string text = "작업 토큰 " + Number(sum);
            if (root != null)
            {
                text += " · 메인 대화 누적 " + Number(root.Total?.TotalTokens);
                text += root.ContextPercent is double percent
                    ? " · 메인 컨텍스트 " + percent.ToString("0.#", CultureInfo.InvariantCulture) + "%"
                    : " · 메인 컨텍스트 미측정";
            }
            foreach (var worker in workers) text += " · " + worker.Label + " " + Number(worker.Total?.TotalTokens);
            return new(text, sum, hasUsage, root, Array.AsReadOnly(workers));
        }
    }

    private static string Number(long? value) => value?.ToString("N0", CultureInfo.InvariantCulture) ?? "미측정/불완전";

    private ThreadState Get(string id)
    {
        if (!_threads.TryGetValue(id, out var state)) _threads[id] = state = new ThreadState();
        return state;
    }

    private CodexThreadUsage Copy(string id)
    {
        ThreadState state = Get(id);
        long? work = !state.Invalid && state.Observed && state.Baseline.HasValue && state.Total != null
            && state.Total.TotalTokens >= state.Baseline.Value ? state.Total.TotalTokens - state.Baseline.Value : null;
        return new(id, state.Label, state.Total, state.Last, work, state.Window);
    }

    private static bool Decreased(CodexTokenCounts a, CodexTokenCounts b) =>
        a.TotalTokens < b.TotalTokens || a.InputTokens < b.InputTokens || a.CachedInputTokens < b.CachedInputTokens
        || a.CacheWriteInputTokens < b.CacheWriteInputTokens || a.OutputTokens < b.OutputTokens
        || a.ReasoningOutputTokens < b.ReasoningOutputTokens;

    private static bool TryCounts(JsonElement json, out CodexTokenCounts? counts)
    {
        counts = null;
        if (json.ValueKind != JsonValueKind.Object) return false;
        string[] names = ["totalTokens", "inputTokens", "cachedInputTokens", "cacheWriteInputTokens", "outputTokens", "reasoningOutputTokens"];
        long[] values = new long[names.Length];
        for (int i = 0; i < names.Length; i++)
            if (!json.TryGetProperty(names[i], out var value) || value.ValueKind != JsonValueKind.Number
                || !value.TryGetInt64(out values[i]) || values[i] < 0) return false;
        counts = new(values[0], values[1], values[2], values[3], values[4], values[5]);
        return true;
    }
}
