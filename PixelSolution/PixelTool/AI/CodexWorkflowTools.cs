using System.Text.RegularExpressions;

namespace PixelTool;

internal static class CodexWorkflowTools
{
    internal static object[] Definitions => new[]
    {
        Tool("begin_project_work", "Register exact project-relative paths before editing. Use simple for a direct edit, scoped for folders or expanded scope, and middle/planned when the user requests a plan or an unresolved design decision requires one. Registration does not show an approval dialog. Do not run a classification or planning pipeline by default. Keep roles empty unless the user requests delegation. Host path and hash safeguards still apply.", new Dictionary<string, object>
        {
            ["mode"] = new { type = "string", @enum = new[] { "simple", "middle", "scoped", "planned" } },
            ["proposalId"] = new { type = "string", maxLength = 40, description = "Host-issued ID only when using a separate planner. Empty for manager-authored Small/Middle/Big plans and scoped work." },
            ["managerReview"] = new { type = "string", maxLength = 1500, description = "Required only when submitting a separate planner proposal; otherwise empty." },
            ["summary"] = new { type = "string", minLength = 1, maxLength = 500 },
            ["reason"] = new { type = "string", minLength = 1, maxLength = 500 },
            ["plan"] = new { type = "string", minLength = 1, maxLength = 3000, description = "Brief intended edit; use numbered steps only for a requested plan." },
            ["paths"] = new { type = "array", maxItems = 20, items = new { type = "string", maxLength = 240 } },
            ["roles"] = new { type = "array", maxItems = 4, uniqueItems = true, items = new { type = "string", @enum = new[] { "programmer", "researcher", "reviewer", "artist" } } }
        }),
        Tool("planner_task", "Run a distinct persistent read-only planner conversation before planned implementation. draft: pass a focused feature request and manager capability assessment grounded in current project evidence. No full transcript. revise: use the same planner with exact revision feedback; task/context must be empty. Review the structured proposal, then register its proposalId and managerReview with begin_project_work. Registration starts implementation without an approval dialog. Maximum 8 phases/request; no writes, editor control, user questions or delegation. No fallback conversation/model on failure.", new Dictionary<string, object>
        {
            ["phase"] = new { type = "string", @enum = new[] { "draft", "revise" } },
            ["task"] = new { type = "string", maxLength = 3000 },
            ["context"] = new { type = "string", maxLength = 6000 }
        }),
        Tool("programmer_task", "Run the dedicated programmer in one persistent isolated CLI conversation for this request. First analyze: inspect code read-only and return findings to the manager, allowed before begin_project_work. The manager judges those findings and registers the exact scope with programmer role, then calls implement with concrete instructions. A registered planner proposal already includes that role and its paths; reuse it. Implement reuses that SAME conversation and can change only declared paths through permission/hash-checked project tools. No approval dialog is shown. The programmer cannot ask the user, use editor controls or delegate. Manager owns reimport, log verification and completion record. Maximum 8 phases/request, one active programmer, 5 minutes/40 tool calls each. If unavailable/failing, report the selected-model/CLI blocker without model/session fallback.", new Dictionary<string, object>
        {
            ["phase"] = new { type = "string", @enum = new[] { "analyze", "implement" } },
            ["task"] = new { type = "string", minLength = 1, maxLength = 3000 },
            ["context"] = new { type = "string", maxLength = 6000 }
        }),
        Tool("delegate_project_task", "Run one real independent read-only CLI conversation for a registered researcher, reviewer or artist role. The artist inspects existing visual assets and proposes an art direction, asset brief and ready-to-use image prompts; it cannot generate image files. Register the read-only role in a scoped or planned work item first. A helper role alone does not require gameplay planning. Pass a focused task and minimum relevant context, not full transcripts. Workers cannot write, control the editor, delegate or question the user. Manager combines findings/questions and directs the programmer for code changes, then verifies. At most 2 concurrent / 4 total calls per request. Wait for all workers before changes or completion. Do not use for trivial requests or play QA.", new Dictionary<string, object>
        {
            ["role"] = new { type = "string", @enum = new[] { "researcher", "reviewer", "artist" } },
            ["task"] = new { type = "string", minLength = 1, maxLength = 3000 },
            ["context"] = new { type = "string", maxLength = 6000 }
        }),
        Tool("complete_project_work", "Only when the user explicitly requests a validated completion record, save concise project memory in Editor/AIWorkLog.md: what changed/was decided, why, and only verified facts. Host checks actual changed paths and validation logs. Do not call as part of the normal change pipeline. Use single-line plain text. Requires editor file-edit permission; report saved=false honestly.", new Dictionary<string, object>
        {
            ["summary"] = new { type = "string", minLength = 1, maxLength = 500 },
            ["reason"] = new { type = "string", minLength = 1, maxLength = 500 }
        })
    };

    // Keep model/user text intact: numbers in prose, paths, versions and Markdown are not list boundaries.
    internal static string FormatApproval(CodexWorkSession.Plan plan, string managerReview = "") => string.Join("\n\n", new[]
    {
        plan.Mode == "scoped" ? "작업 범위 승인 (게임 기획 변경 없음)" : "작업 계획 승인",
        "요약\n" + plan.Summary,
        "이유\n" + plan.Reason,
        "대상\n" + (plan.Paths.Length == 0 ? "파일 변경 없음" : string.Join("\n", plan.Paths.Select(path => "- " + path))),
        "역할\n- 관리자 (판단·검증·기록)" + (plan.ProposalId.Length == 0 ? "" : "\n- 기획자 (읽기 전용 설계·부분 재기획)") + string.Concat(plan.Roles.Select(role => "\n- " + (role switch { "programmer" => "플머", "artist" => "아티스트", "researcher" => "조사자", "reviewer" => "검토자", _ => role }) + (role == "programmer" ? " (분석·수정)" : " (읽기 전용)"))),
        "작업 순서\n" + plan.Details + (managerReview.Length == 0 ? "" : "\n\n관리자 검토\n" + managerReview + "\n\n수정하려면 항목 번호와 내용을 입력하세요. 예: 2: 중력과 점프 방식을 변경"),
        "파일 경로·해시 검사와 필요한 계획 승인은 유지됩니다."
    });

    // Presentation only: the approval state and complete scope always use the original plan.
    internal static string SingleLine(string text) => Regex.Replace(text, @"\s+", " ").Trim();

    internal static string ApprovalPreview(CodexWorkSession.Plan plan, CodexPlanProposal? proposal)
    {
        string steps = plan.Mode == "planned" && proposal?.Id == plan.ProposalId
            ? string.Join("\n\n", proposal.Sections.Select(section => $"{section.Number}. {SingleLine(section.Title)}\n   내용: {BriefStep(section.Design)}"))
            : NumberedSteps(plan.Details);
        return (plan.Mode == "scoped" ? "작업 범위 승인 · 기존 기획 유지" : "작업 계획 승인") +
            "\n" + SingleLine(plan.Summary) + "\n\n작업 순서\n" + steps +
            "\n\n승인 범위·제약·검증은 상세 보기에서 확인할 수 있습니다." +
            (plan.Mode == "planned" ? "\n수정: 항목 번호와 내용 (예: 2: 점프 방식 변경)" : "");
    }

    private static string BriefStep(string design)
    {
        string firstLine = SingleLine(design.Trim().Split(new[] { '\r', '\n' }, 2)[0]);
        int sentenceEnd = firstLine.IndexOf(". ", StringComparison.Ordinal);
        if (sentenceEnd >= 0) firstLine = firstLine[..(sentenceEnd + 1)];
        if (firstLine.Length <= 140) return firstLine;
        int boundary = firstLine.LastIndexOf(' ', 139);
        return firstLine[..(boundary > 40 ? boundary : 140)].TrimEnd() + "…";
    }

    internal static string NumberedSteps(string text)
    {
        // Recognize only consecutive, line-start list markers. Never split decimal values,
        // filenames or inline prose; the complete original remains in the approval details.
        string[] lines = text.Replace("\r\n", "\n").Split('\n').Where(line => !string.IsNullOrWhiteSpace(line)).ToArray();
        var steps = new List<string>();
        foreach (string line in lines)
        {
            var match = Regex.Match(line, @"^\s*([0-9]{1,2})[.)](?![0-9])\s*(.+)$");
            if (match.Success && int.Parse(match.Groups[1].Value) == steps.Count + 1)
                steps.Add(SingleLine(match.Groups[2].Value));
            else if (steps.Count > 0)
                steps[^1] += " " + SingleLine(line);
            else
                return "1. " + SingleLine(text);
        }
        return string.Join("\n\n", steps.Select((step, index) => $"{index + 1}. {step}"));
    }

    internal static string DispatchMessage(string role, string phase, string task)
    {
        (string name, string activity) = role switch
        {
            "programmer" => ("플머", phase == "implement" ? "구현 중" : "분석 중"),
            "planner" => ("기획자", phase == "revise" ? "지정 항목 수정 중" : "계획 작성 중"),
            "researcher" => ("조사자", "자료 확인 중"),
            "reviewer" => ("검토자", "검토 중"),
            "artist" => ("아티스트", "비주얼 기획 중"),
            _ => ("작업자", "작업 중")
        };
        // A short first-line subject is optional; legacy single-line instructions stay private.
        string subject = task.Contains('\n') ? SingleLine(task.Split('\n')[0]) : "";
        if (role == "planner" || subject.Length > 32) subject = "";
        return name + ": " + (subject.Length == 0 ? "" : subject + " ") + activity;
    }

    private static object Tool(string name, string description, Dictionary<string, object> properties) => new
    {
        type = "function", name, description, strict = true,
        parameters = new { type = "object", properties, required = properties.Keys.ToArray(), additionalProperties = false }
    };

    // Tool instructions stay short; archived AI documents are not loaded.
    internal const string Instructions =
        "\nHandle the user's request directly without a classification, planning, or delegation pipeline. " +
        "Before editing, register exact paths with begin_project_work(mode=simple, roles=[]); use scoped or a plan only when required by the host or explicitly requested. " +
        "Use Lua API documents only when relevant and verify current Engine declarations. Preserve path, hash, permission, and unsaved-buffer checks. " +
        "If a file or folder creation/edit is blocked because it targets outside Asset, reply with only this sentence: " + ProjectFileAccess.AssetMutationDenied + " Do not suggest manual creation, right-click steps, alternate locations, or a workaround. " +
        "Do not automatically reimport, inspect logs, run gameplay checks, or call complete_project_work; perform validation when requested. " +
        "Report actual changed files and unrun checks honestly. Never invent measurements or tests.";
}
