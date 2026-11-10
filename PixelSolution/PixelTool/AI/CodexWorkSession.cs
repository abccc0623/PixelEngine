using System.Diagnostics;
using System.IO;
using System.Text.Json;

namespace PixelTool;

// One request's approved scope. Only the manager writes; worker sessions are read-only.
internal sealed class CodexWorkSession(string project, long? logCursor, bool requirePlan = false)
{
    private readonly ProjectFileAccess _files = new(project);
    private readonly object _gate = new();
    private readonly HashSet<string> _scope = new(StringComparer.OrdinalIgnoreCase);
    private readonly HashSet<string> _changed = new(StringComparer.OrdinalIgnoreCase);
    private readonly HashSet<string> _roles = new(StringComparer.Ordinal);
    private int _workers, _workerCalls, _failedChecks;
    private bool _declared, _planned, _finished, _needsReimport, _mutating;
    private bool _planRequired = requirePlan;
    private long? _checkCursor = logCursor;
    internal string Id { get; } = Guid.NewGuid().ToString("N");
    internal Stopwatch Clock { get; } = Stopwatch.StartNew();
    internal bool Finished { get { lock (_gate) return _finished; } }
    internal bool HasChanges { get { lock (_gate) return _changed.Count > 0; } }
    internal static bool IsMutation(string name) => name is "write_project_file" or "patch_project_file" or
        "delete_asset_file" or "delete_asset_folder" or "move_asset" or "create_asset_folder" or
        "create_png_asset" or "copy_asset" or "import_asset" or "reimport_project";

    internal sealed record Plan(string Mode, string Summary, string Reason, string Details, string[] Paths, string[] Roles);
    internal Plan ValidatePlan(JsonElement args)
    {
        var plan = new Plan(Text(args, "mode", 20), Text(args, "summary", 500), Text(args, "reason", 500),
            Text(args, "plan", 3000), args.GetProperty("paths").EnumerateArray().Select(p => Canonical(p.GetString() ?? "")).Distinct(StringComparer.OrdinalIgnoreCase).ToArray(),
            args.GetProperty("roles").EnumerateArray().Select(p => p.GetString() ?? "").Distinct().ToArray());
        if (plan.Mode is not ("simple" or "planned") || plan.Paths.Length > 20 || plan.Roles.Any(r => r is not ("researcher" or "reviewer")))
            throw new InvalidOperationException("작업 유형·대상 파일·담당 역할을 확인하세요.");
        if (plan.Mode == "simple" && (_planRequired || plan.Paths.Length > 2 || plan.Roles.Length != 0 || plan.Paths.Any(p => Directory.Exists(_files.Resolve(p)))))
            throw new InvalidOperationException("간단 작업은 최대 두 파일만 처리합니다. 여러 파일·폴더·보조 역할이 필요하면 planned 계획 승인을 받으세요.");
        lock (_gate)
        {
            if (_finished || _workers != 0) throw new InvalidOperationException("완료했거나 보조 작업이 실행 중인 작업의 계획은 변경할 수 없습니다.");
            if (_declared && plan.Mode == "simple" && (!_scope.SetEquals(plan.Paths) || _planned))
                throw new InvalidOperationException("진행 중 범위 변경은 planned 계획을 다시 승인받아야 합니다.");
            if (_changed.Any(p => !plan.Paths.Contains(p, StringComparer.OrdinalIgnoreCase)))
                throw new InvalidOperationException("새 계획에는 이미 변경한 경로도 포함해야 합니다.");
        }
        return plan;
    }

    internal void PrepareApproval() { lock (_gate) { _declared = false; _planRequired = true; } }

    internal void AcceptPlan(Plan plan)
    {
        lock (_gate)
        {
            if (_finished || _workers != 0) throw new InvalidOperationException("작업 상태가 변경되었습니다.");
            _scope.Clear(); _scope.UnionWith(plan.Paths);
            _roles.Clear(); _roles.UnionWith(plan.Roles);
            _declared = true; _planned = plan.Mode == "planned";
        }
    }

    internal void CheckMutation(string name, JsonElement args)
    {
        lock (_gate)
        {
            if (!_declared || _finished) throw new InvalidOperationException("수정 전에 begin_project_work로 작업 범위를 등록하세요. 큰 작업은 사용자 계획 승인이 필요합니다.");
            if (_workers != 0) throw new InvalidOperationException("보조 작업을 마친 뒤 관리자가 변경하세요. 검토 중 파일을 바꾸지 않습니다.");
            if (_failedChecks >= 3) throw new InvalidOperationException("검증 실패가 세 번 반복되어 수정을 중단했습니다. 현재 오류와 필요한 결정을 사용자에게 알려 주세요.");
            if (name == "reimport_project") { _mutating = true; return; }
            string path = Canonical(args.GetProperty("path").GetString() ?? "");
            if (!_scope.Contains(path)) throw new InvalidOperationException("승인된 작업 범위 밖입니다. 실제 경로를 확인하고 계획을 다시 승인받으세요.");
            if (!_planned && (name is "delete_asset_folder" or "create_asset_folder" || Directory.Exists(_files.Resolve(path))))
                throw new InvalidOperationException("폴더 변경은 planned 계획 승인이 필요합니다.");
            if (!_planned && !Directory.Exists(Path.GetDirectoryName(_files.Resolve(path))))
                throw new InvalidOperationException("새 부모 폴더가 필요한 파일은 planned 계획 승인을 받아야 합니다.");
            if (name == "move_asset" && !_scope.Contains(Canonical(args.GetProperty("destination").GetString() ?? "")))
                throw new InvalidOperationException("이동 목적지도 승인된 계획에 포함해야 합니다.");
            _mutating = true;
        }
    }

    internal void EndMutation() { lock (_gate) _mutating = false; }

    internal void TrackResult(string name, string result)
    {
        if (!IsMutation(name) || name == "reimport_project" || !result.TrimStart().StartsWith('{')) return;
        using var json = JsonDocument.Parse(result);
        lock (_gate)
        {
            foreach (string key in new[] { "saved", "created", "moved", "destination", "deleted" })
                if (json.RootElement.TryGetProperty(key, out var value) && value.ValueKind == JsonValueKind.String)
                {
                    string path = Canonical(value.GetString()!);
                    _changed.Add(path);
                    if (path.StartsWith("Asset/", StringComparison.OrdinalIgnoreCase)) _needsReimport = true;
                }
        }
    }

    internal void Reimported(JsonElement result)
    {
        lock (_gate)
            if (result.TryGetProperty("reimportCompleted", out var done) && done.ValueKind == JsonValueKind.True &&
                result.TryGetProperty("afterCursor", out var cursor) && cursor.TryGetInt64(out long value) && value >= 0)
            { _checkCursor = value; _needsReimport = false; }
    }

    internal void StartWorker(string role)
    {
        lock (_gate)
        {
            if (!_declared || !_planned || _finished || !_roles.Contains(role))
                throw new InvalidOperationException("이 역할은 승인된 계획에 없습니다. 관리자 계획 승인 후 필요한 보조 작업만 요청하세요.");
            if (_mutating) throw new InvalidOperationException("관리자 변경 작업이 끝난 뒤 보조 작업을 시작하세요.");
            if (_workers >= 2 || _workerCalls >= 4) throw new InvalidOperationException("동시 보조 작업은 두 개, 요청당 호출은 네 번까지입니다. 결과를 정리해 관리자가 이어서 처리하세요.");
            _workers++; _workerCalls++;
        }
    }
    internal void EndWorker() { lock (_gate) _workers--; }

    internal ProjectWorkLog.Result Complete(string summary, string reason, Func<long, int, JsonElement>? readLogs, long? tokens)
    {
        lock (_gate)
        {
            if (!_declared || _workers != 0) throw new InvalidOperationException("작업 범위를 등록하고 보조 작업 결과를 모두 모은 뒤 완료하세요.");
            if (_finished) return new(true, ProjectWorkLog.RelativePath, "이 작업의 기억은 이미 저장했습니다.");
            if (_needsReimport) throw new InvalidOperationException("Asset 변경 후 리임포트를 완료하고 오류를 확인해야 합니다.");
            if (readLogs == null || _checkCursor == null) throw new InvalidOperationException("작업 시작 로그 기준을 확보하지 못했습니다. 완료로 기록하지 말고 확인 불가를 알리세요.");
            long cursor = _checkCursor.Value;
            var errors = new List<string>();
            bool complete = false;
            for (int page = 0; page < 20; page++)
            {
                var logs = readLogs(cursor, 100);
                if (logs.GetProperty("cleared").GetBoolean() || logs.GetProperty("truncated").GetBoolean())
                    throw new InvalidOperationException("검증 로그가 삭제되었거나 누락되었습니다. 성공으로 기록하지 않습니다.");
                foreach (var entry in logs.GetProperty("entries").EnumerateArray())
                    if (entry.GetProperty("level").GetInt32() >= 2 && errors.Count < 5)
                        errors.Add(entry.GetProperty("message").GetString() ?? "오류 내용 없음");
                long next = logs.GetProperty("cursor").GetInt64();
                if (!logs.GetProperty("hasMore").GetBoolean()) { complete = true; break; }
                if (next <= cursor) break;
                cursor = next;
            }
            if (!complete) throw new InvalidOperationException("로그 페이지를 모두 확인하지 못했습니다. 완료로 기록하지 않습니다.");
            if (errors.Count != 0)
            {
                _failedChecks++;
                throw new InvalidOperationException("새 오류가 남아 있습니다. 요청 범위 안에서 원인을 수정·리임포트 후 재확인하세요. 반복 한도 3회.\n" + string.Join("\n", errors));
            }
            string validation = "현재 검증 구간의 Tool 오류 로그 없음. 전체 플레이 QA는 수행하지 않음.";
            var saved = ProjectWorkLog.Append(project, new(Id, summary, reason, _changed.Order().ToArray(), validation,
                DateTimeOffset.UtcNow, Clock.Elapsed, tokens));
            if (saved.Saved) _finished = true;
            return saved;
        }
    }

    private string Canonical(string path)
    {
        if (string.IsNullOrWhiteSpace(path) || path.Length > 240) throw new InvalidOperationException("짧고 정확한 프로젝트 상대 경로가 필요합니다.");
        string value = Path.GetRelativePath(_files.Root, _files.Resolve(path)).Replace('\\', '/');
        if (!value.StartsWith("Asset/", StringComparison.OrdinalIgnoreCase) && !value.Equals(ProjectNavigation.NotesPath, StringComparison.OrdinalIgnoreCase))
            throw new InvalidOperationException("작업 범위는 Asset 내부 또는 Editor/AINavigation.md만 지정할 수 있습니다.");
        return value;
    }
    internal static string Text(JsonElement args, string key, int max)
    {
        string value = args.GetProperty(key).GetString()?.Trim() ?? "";
        if (string.IsNullOrEmpty(value) || value.Length > max) throw new InvalidOperationException($"{key}는 1~{max}자로 작성하세요.");
        return value;
    }
}
