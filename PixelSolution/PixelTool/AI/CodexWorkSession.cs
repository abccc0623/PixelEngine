using System.Diagnostics;
using System.IO;
using System.Text.Json;

namespace PixelTool;

// One request's scope: the user approves planned work; the programmer uses the same guarded tools.
internal sealed class CodexWorkSession(string project, long? logCursor, bool requirePlan = false)
{
    private readonly ProjectFileAccess _files = new(project);
    private readonly object _gate = new();
    private readonly HashSet<string> _scope = new(StringComparer.OrdinalIgnoreCase);
    private readonly HashSet<string> _changed = new(StringComparer.OrdinalIgnoreCase);
    private readonly HashSet<string> _roles = new(StringComparer.Ordinal);
    private int _workers, _workerCalls, _failedChecks, _programmerCalls;
    private bool _programmerRunning, _programmerImplementing, _programmerAnalyzed;
    private bool _declared, _scopeApproved, _finished, _needsReimport, _mutating;
    private bool _planRequired = requirePlan;
    private bool _plannerRunning, _canceled, _approvalPending, _scopeApprovalRequired, _designApproved;
    private Plan? _acceptedPlan, _approvedDesign, _pendingScope;
    private int _plannerCalls;
    internal CodexPlanProposal? Proposal { get; private set; }
    internal CodexPlanProposal.Revision? PendingRevision { get; private set; }
    private long? _checkCursor = logCursor;
    internal string Id { get; } = Guid.NewGuid().ToString("N");
    internal Stopwatch Clock { get; } = Stopwatch.StartNew();
    internal bool Finished { get { lock (_gate) return _finished; } }
    internal bool HasChanges { get { lock (_gate) return _changed.Count > 0; } }
    internal static bool IsMutation(string name) => name is "write_project_file" or "patch_project_file" or
        "delete_asset_file" or "delete_asset_folder" or "move_asset" or "create_asset_folder" or
        "create_png_asset" or "copy_asset" or "import_asset" or "reimport_project";

    internal sealed record Plan(string Mode, string Summary, string Reason, string Details, string[] Paths, string[] Roles, string ProposalId = "");
    internal Plan ValidatePlan(JsonElement args)
    {
        var plan = new Plan(Text(args, "mode", 20), Text(args, "summary", 500), Text(args, "reason", 500),
            Text(args, "plan", 3000), args.GetProperty("paths").EnumerateArray().Select(p => Canonical(p.GetString() ?? "")).Distinct(StringComparer.OrdinalIgnoreCase).ToArray(),
            args.GetProperty("roles").EnumerateArray().Select(p => p.GetString() ?? "").Distinct().ToArray());
        if (plan.Mode is not ("simple" or "middle" or "scoped" or "planned") || plan.Paths.Length > 20 || plan.Roles.Any(r => r is not ("researcher" or "reviewer" or "artist" or "programmer")))
            throw new InvalidOperationException("작업 유형·대상 파일·담당 역할을 확인하세요.");
        if (plan.Mode == "simple" && (_planRequired || _scopeApprovalRequired || plan.Roles.Length != 0 || plan.Paths.Any(p => Directory.Exists(_files.Resolve(p)))))
            throw new InvalidOperationException("기획 결정이 남거나 사용자가 기획을 요청하면 planned를 사용하세요. 명확한 폴더·추가 역할 작업은 scoped 범위 승인이 필요합니다.");
        if (plan.Mode == "scoped" && (_planRequired && !_designApproved || PendingRevision != null))
            throw new InvalidOperationException("요청된 기획 승인을 범위 승인으로 대신할 수 없습니다.");
        if (plan.Mode == "scoped") plan = plan with { ProposalId = Proposal?.Id ?? "" };
        lock (_gate)
        {
            if (_canceled || _finished || _workers != 0) throw new InvalidOperationException("완료했거나 보조 작업이 실행 중인 작업의 계획은 변경할 수 없습니다.");
            if (_declared && plan.Mode == "simple" && (!_scope.SetEquals(plan.Paths) || _scopeApproved))
                throw new InvalidOperationException("진행 중 경로 범위 변경은 scoped 범위 승인을 받으세요. 동작 기획까지 바뀌면 planned가 필요합니다.");
            if (_changed.Any(p => !plan.Paths.Contains(p, StringComparer.OrdinalIgnoreCase)) && !IsScopeApproved(plan))
                throw new InvalidOperationException("새 계획에는 이미 변경한 경로도 포함해야 합니다.");
        }
        return plan;
    }

    internal void PrepareApproval(string? proposalId = null) { lock (_gate) { if (_canceled || _workers != 0 || _mutating || (proposalId != null && Proposal?.Id != proposalId)) throw new InvalidOperationException("현재 작업 상태에서는 승인을 요청할 수 없습니다."); _declared = false; _planRequired = true; _approvalPending = true; } }

    internal void StartPlanner(string phase)
    {
        lock (_gate)
        {
            if (_canceled || _finished || _approvalPending || _mutating || _workers != 0 || _plannerCalls >= 8 ||
                phase is not ("draft" or "revise") || (phase == "revise" && (Proposal == null || PendingRevision == null)) ||
                (phase == "draft" && PendingRevision != null))
                throw new InvalidOperationException("기획자는 한 번에 한 단계씩, 요청당 최대 여덟 번 실행합니다. 수정 요청이 있으면 같은 기획자의 revise 단계로 전달하세요.");
            _declared = false; _planRequired = true; _designApproved = false;
            if (phase == "draft") Proposal = null;
            _plannerRunning = true; _workers++; _plannerCalls++;
        }
    }

    internal CodexPlanProposal StoreProposal(string report, string phase)
    {
        lock (_gate)
        {
            if (_canceled || !_plannerRunning) throw new InvalidOperationException("현재 실행 중인 기획 결과만 받을 수 있습니다.");
            var proposal = CodexPlanProposal.Parse(report, phase == "revise" ? Proposal : null, phase == "revise" ? PendingRevision : null);
            proposal = proposal with { Sections = proposal.Sections.Select(section =>
            {
                var paths = section.Paths.Select(Canonical).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
                return section.Paths.SequenceEqual(paths) ? section : section with { Paths = paths };
            }).ToArray() };
            if (_changed.Any(p => !proposal.Paths.Contains(p, StringComparer.OrdinalIgnoreCase)))
                throw new InvalidOperationException("새 기획안은 이미 변경한 경로를 포함해야 합니다.");
            Proposal = proposal; PendingRevision = null;
            return proposal;
        }
    }

    internal void EndPlanner() { lock (_gate) { if (_plannerRunning) { _plannerRunning = false; _workers--; } } }

    internal Plan ManagerPlanForReview(JsonElement args)
    {
        if (Proposal != null || PendingRevision != null || args.GetProperty("proposalId").GetString() != "")
            throw new InvalidOperationException("별도 기획자의 기획안은 발급된 proposalId로 제출하세요.");
        var plan = ValidatePlan(args);
        if (plan.Mode is not ("middle" or "planned"))
            throw new InvalidOperationException("간단한 기획은 middle, 상세 기획은 planned를 사용하세요.");
        return plan;
    }

    internal Plan PlanForReview(JsonElement args)
    {
        lock (_gate)
        {
            if (_canceled || _finished || _workers != 0 || Proposal == null || PendingRevision != null ||
                Text(args, "proposalId", 40) != Proposal.Id)
                throw new InvalidOperationException("현재 기획자의 최신 기획안을 먼저 검토하세요. 수정 요청 중이거나 오래된 기획안은 승인할 수 없습니다.");
            Text(args, "managerReview", 1500);
            // Repeating the same reviewed design must not narrow a later approved execution scope,
            // including paths already changed after an operational extension.
            var approvedDesign = new Plan("planned", Proposal.Summary, Proposal.Reason, Proposal.Details,
                Proposal.Paths, Proposal.Roles, Proposal.Id);
            if (IsPlanApproved(approvedDesign)) return approvedDesign;
            var plan = ValidatePlan(JsonSerializer.SerializeToElement(new
            { mode = "planned", summary = Proposal.Summary, reason = Proposal.Reason, plan = "구조화된 기획안",
                paths = Proposal.Paths, roles = Proposal.Roles }));
            return plan with { Details = Proposal.Details, ProposalId = Proposal.Id };
        }
    }

    internal bool IsPlanApproved(Plan plan) { lock (_gate) return !_canceled && !_finished && !_approvalPending &&
        _declared && _scopeApproved && _designApproved && PendingRevision == null &&
        plan.ProposalId == (Proposal?.Id ?? "") && SamePlan(plan, _approvedDesign); }

    // Operational permission is separate from gameplay design. It cannot waive requested planning.
    internal void PrepareScopeApproval(Plan plan)
    {
        lock (_gate)
        {
            if (_canceled || _finished || _workers != 0 || _mutating || _approvalPending ||
                plan.Mode != "scoped" || (_planRequired && !_designApproved) || PendingRevision != null ||
                plan.ProposalId != (Proposal?.Id ?? ""))
                throw new InvalidOperationException("현재 작업은 범위 승인으로 진행할 수 없습니다.");
            _declared = false; _scopeApprovalRequired = true; _approvalPending = true; _pendingScope = plan;
        }
    }
    internal void ReviseScope() { lock (_gate) { _approvalPending = false; _pendingScope = null; } }
    internal bool IsScopeApproved(Plan plan) { lock (_gate) return !_canceled && !_finished && !_approvalPending &&
        _declared && _scopeApproved && PendingRevision == null && plan.Mode == "scoped" &&
        (SamePlan(plan, _acceptedPlan) ||
         // The stored approved design still governs behavior. A scoped handoff cannot replace it
         // or grant more paths/roles merely by restating the implementation instructions.
         (_designApproved && _approvedDesign != null && (Proposal?.Id ?? "") == plan.ProposalId &&
          _approvedDesign.ProposalId == plan.ProposalId && _scope.IsSupersetOf(plan.Paths) && _roles.IsSupersetOf(plan.Roles))); }
    private static bool SamePlan(Plan plan, Plan? other) => other != null &&
        plan.Mode == other.Mode && plan.ProposalId == other.ProposalId && plan.Summary == other.Summary &&
        plan.Reason == other.Reason && plan.Details == other.Details &&
        plan.Paths.ToHashSet(StringComparer.OrdinalIgnoreCase).SetEquals(other.Paths) && plan.Roles.ToHashSet().SetEquals(other.Roles);

    internal void RequestRevision(string answer, string proposalId)
    {
        lock (_gate)
        {
            if (_canceled || Proposal == null || Proposal.Id != proposalId || _workers != 0) throw new InvalidOperationException("표시한 기획안이 현재 기획안과 다릅니다. 최신 기획안을 확인하세요.");
            var revision = Proposal.ParseRevision(answer);
            _declared = false; _planRequired = true; _designApproved = false; _approvalPending = false; PendingRevision = revision;
        }
    }

    internal void ReviseManagerPlan() { lock (_gate) { _approvalPending = false; _declared = false; _designApproved = false; } }

    internal void CancelPlan() { lock (_gate) { _canceled = true; _declared = false; _approvalPending = false; } }

    internal Plan AcceptPlan(Plan plan)
    {
        lock (_gate)
        {
            if (_canceled || _finished || _workers != 0) throw new InvalidOperationException("작업 상태가 변경되었습니다.");
            if (IsPlanApproved(plan) || IsScopeApproved(plan)) return _acceptedPlan!;
            if (plan.Mode == "simple" && (_planRequired || _scopeApprovalRequired) ||
                plan.Mode == "scoped" && ((_planRequired && !_designApproved) || PendingRevision != null))
                throw new InvalidOperationException("계획 또는 작업 범위 승인 상태가 변경되었습니다.");
            if ((plan.ProposalId.Length > 0 && Proposal == null) || (Proposal != null && ((plan.Mode != "planned" && !(plan.Mode == "scoped" && _designApproved)) || plan.ProposalId != Proposal.Id || PendingRevision != null)))
                throw new InvalidOperationException("현재 기획안만 승인할 수 있습니다.");
            if (plan.Mode == "scoped" && !SamePlan(plan, _pendingScope) && !IsScopeApproved(plan))
                throw new InvalidOperationException("표시한 작업 범위만 승인할 수 있습니다.");
            _scope.Clear(); _scope.UnionWith(plan.Paths);
            _roles.Clear(); _roles.UnionWith(plan.Roles);
            _declared = true; _scopeApproved = plan.Mode != "simple"; _approvalPending = false;
            if (plan.Mode is "middle" or "planned") { _designApproved = true; _approvedDesign = plan; }
            _scopeApprovalRequired = false; _pendingScope = null; _acceptedPlan = plan;
            return plan;
        }
    }

    internal void CheckMutation(string name, JsonElement args, bool programmer = false)
    {
        lock (_gate)
        {
            if (_canceled || !_declared || _finished) throw new InvalidOperationException("수정 전에 begin_project_work로 작업 범위를 등록하세요. 기획이 필요한 작업은 사용자 기획 승인이 필요합니다.");
            if (programmer ? !_programmerRunning || !_programmerImplementing || _workers != 1 :
                _workers != 0 || (_roles.Contains("programmer") && name != "reimport_project"))
                throw new InvalidOperationException("승인된 직접 작업 또는 프로그래머 구현 단계에서만 범위 안의 파일을 수정할 수 있습니다.");
            if (_failedChecks >= 3) throw new InvalidOperationException("검증 실패가 세 번 반복되어 수정을 중단했습니다. 현재 오류와 필요한 결정을 사용자에게 알려 주세요.");
            if (name == "reimport_project") { _mutating = true; return; }
            string path = Canonical(args.GetProperty("path").GetString() ?? "");
            if (!_scope.Contains(path)) throw new InvalidOperationException("승인된 작업 범위 밖입니다. 실제 경로를 확인하고 계획을 다시 승인받으세요.");
            if (!_scopeApproved && (name is "delete_asset_folder" or "create_asset_folder" || Directory.Exists(_files.Resolve(path))))
                throw new InvalidOperationException("폴더 변경은 scoped 작업 범위 승인 또는 planned 기획 승인이 필요합니다.");
            if (!_scopeApproved && !Directory.Exists(Path.GetDirectoryName(_files.Resolve(path))))
                throw new InvalidOperationException("새 부모 폴더가 필요한 파일은 scoped 작업 범위 승인 또는 planned 기획 승인이 필요합니다.");
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
            if (_canceled || !_declared || !_scopeApproved || _finished || role is not ("researcher" or "reviewer" or "artist") || !_roles.Contains(role))
                throw new InvalidOperationException("이 역할은 승인된 범위에 없습니다. scoped 범위 승인 또는 planned 기획 승인 후 필요한 보조 작업만 요청하세요.");
            if (_mutating || _programmerRunning) throw new InvalidOperationException("수정 또는 프로그래머 작업이 끝난 뒤 보조 작업을 시작하세요.");
            if (_workers >= 2 || _workerCalls >= 4) throw new InvalidOperationException("동시 보조 작업은 두 개, 요청당 호출은 네 번까지입니다. 결과를 정리해 관리자가 결과를 정리하세요.");
            _workers++; _workerCalls++;
        }
    }
    internal void EndWorker() { lock (_gate) _workers--; }

    internal void StartProgrammer(string phase)
    {
        lock (_gate)
        {
            if (phase is not ("analyze" or "implement") || _canceled || _approvalPending || _finished || _mutating || _workers != 0 || _programmerCalls >= 8)
                throw new InvalidOperationException("프로그래머는 한 번에 한 단계씩, 요청당 최대 여덟 번 실행합니다. 다른 작업이 끝난 뒤 요청하세요.");
            if (phase == "implement" && (!_programmerAnalyzed || !_declared || !_roles.Contains("programmer")))
                throw new InvalidOperationException("프로그래머 분석을 먼저 완료하고, 관리자가 programmer 역할과 정확한 범위를 등록/승인한 뒤 구현을 지시하세요.");
            if (phase == "implement" && _failedChecks >= 3)
                throw new InvalidOperationException("검증 실패가 세 번 반복되었습니다. 수정을 중단하고 원인을 보고하세요.");
            _programmerRunning = true; _programmerImplementing = phase == "implement";
            _workers++; _programmerCalls++;
        }
    }
    internal void EndProgrammer(string phase, bool completed)
    {
        lock (_gate)
        {
            if (!_programmerRunning) return;
            if (phase == "analyze") _programmerAnalyzed = completed;
            _programmerRunning = false; _programmerImplementing = false; _workers--;
        }
    }

    internal ProjectWorkLog.Result Complete(string summary, string reason, Func<long, int, JsonElement>? readLogs, long? tokens)
    {
        lock (_gate)
        {
            if (_canceled || !_declared || _workers != 0) throw new InvalidOperationException("작업 범위를 등록하고 보조 작업 결과를 모두 모은 뒤 완료하세요.");
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
            throw new InvalidOperationException(ProjectFileAccess.AssetMutationDenied);
        return value;
    }
    internal static string Text(JsonElement args, string key, int max)
    {
        string value = args.GetProperty(key).GetString()?.Trim() ?? "";
        if (string.IsNullOrEmpty(value) || value.Length > max) throw new InvalidOperationException($"{key}는 1~{max}자로 작성하세요.");
        return value;
    }
}
