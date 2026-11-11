using System.Text.Json;
using System.Text.RegularExpressions;

namespace PixelTool;

// Structured, host-owned proposals. Models never choose which sections a user revision may replace.
internal sealed record CodexPlanProposal(string Id, string Summary, string Reason, string[] Roles, CodexPlanProposal.Section[] Sections)
{
    internal sealed record Section(int Number, string Title, string Capability, string Design, string Evidence,
        string Verification, string References, string[] Paths);
    internal sealed record Revision(int[] Numbers, string Feedback);
    internal string[] Paths => Sections.SelectMany(s => s.Paths).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
    internal string Details => string.Join("\n\n", Sections.Select(s =>
        $"{s.Number}. {s.Title}\n   엔진 지원·한계: {s.Capability}\n   구현할 동작: {s.Design}\n   근거: {s.Evidence}\n   완료 확인: {s.Verification}\n   참고: {s.References}"));

    internal static CodexPlanProposal Parse(string text, CodexPlanProposal? previous = null, Revision? revision = null)
    {
        if (text.Length > 22000) throw new InvalidOperationException("기획안은 22000자 이내여야 합니다.");
        text = text.Trim();
        if (text.StartsWith("```json", StringComparison.Ordinal)) text = text[7..].Trim();
        else if (text.StartsWith("```", StringComparison.Ordinal)) text = text[3..].Trim();
        if (text.EndsWith("```", StringComparison.Ordinal)) text = text[..^3].Trim();
        using var json = JsonDocument.Parse(text);
        var root = json.RootElement;
        var sections = root.GetProperty("sections").EnumerateArray().Select(s => new Section(
            s.GetProperty("number").GetInt32(), Text(s, "title", 100), Text(s, "capability", 600),
            Text(s, "design", 1200), Text(s, "evidence", 600), Text(s, "verification", 500),
            Text(s, "references", 400), s.GetProperty("paths").EnumerateArray().Select(p => p.GetString() ?? "").ToArray())).ToArray();
        if (sections.Length is < 1 or > 12 || sections.Any(s => s.Number is < 1 or > 12 || s.Paths.Length > 20) ||
            sections.Select(s => s.Number).Distinct().Count() != sections.Length)
            throw new InvalidOperationException("기획 항목은 중복 없는 번호로 최대 12개까지 작성하세요.");
        CodexPlanProposal proposal;
        if (revision != null)
        {
            if (previous == null || !revision.Numbers.ToHashSet().SetEquals(sections.Select(s => s.Number)))
                throw new InvalidOperationException("사용자가 지정한 항목만 다시 기획하세요. 다른 항목의 추가·삭제·수정은 허용하지 않습니다.");
            var replacements = sections.ToDictionary(s => s.Number);
            proposal = previous with { Id = Guid.NewGuid().ToString("N"),
                Sections = previous.Sections.Select(s => replacements.GetValueOrDefault(s.Number, s)).ToArray() };
        }
        else
        {
            if (!sections.Select(s => s.Number).SequenceEqual(Enumerable.Range(1, sections.Length)))
                throw new InvalidOperationException("첫 기획안 항목 번호는 1부터 순서대로 작성하세요.");
            string[] roles = root.GetProperty("roles").EnumerateArray().Select(r => r.GetString() ?? "").Distinct().ToArray();
            if (roles.Any(r => r is not ("programmer" or "researcher" or "reviewer")))
                throw new InvalidOperationException("기획안의 구현·검토 역할을 확인하세요.");
            proposal = new(Guid.NewGuid().ToString("N"), Text(root, "summary", 500), Text(root, "reason", 500), roles, sections);
        }
        if (JsonSerializer.Serialize(proposal, new JsonSerializerOptions { Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping }).Length > 20500)
            throw new InvalidOperationException("기획안의 전체 구조와 경로는 20500자 이내여야 합니다.");
        if (proposal.Details.Length > 16000) throw new InvalidOperationException("기획안 전체 본문은 16000자 이내여야 합니다.");
        if (proposal.Paths.Length > 20) throw new InvalidOperationException("기획안의 정확한 대상 경로는 최대 20개입니다.");
        return proposal;
    }

    internal Revision ParseRevision(string answer)
    {
        if (answer.Length > 1500) throw new InvalidOperationException("수정 요청은 1500자 이내로 입력하세요.");
        var match = Regex.Match(answer, @"^\s*([0-9]{1,2}(?:\s*,\s*[0-9]{1,2})*)\s*[:：]\s*(.+)$", RegexOptions.Singleline);
        if (!match.Success || match.Groups[2].Value.Trim().Length is < 1 or > 1200)
            throw new InvalidOperationException("수정할 번호와 내용을 입력하세요. 예: 1, 3: 이동 입력과 점프 방식 변경");
        int[] numbers = match.Groups[1].Value.Split(',').Select(n => int.Parse(n.Trim())).Distinct().ToArray();
        if (numbers.Any(n => !Sections.Any(s => s.Number == n)))
            throw new InvalidOperationException("현재 기획안에 있는 항목 번호만 선택하세요.");
        return new(numbers, match.Groups[2].Value.Trim());
    }

    internal string RevisionContext(Revision revision) => JsonSerializer.Serialize(new
    {
        requestedSections = revision.Numbers,
        userFeedback = revision.Feedback,
        sections = Sections.Where(s => revision.Numbers.Contains(s.Number)).Select(s => new
        { number = s.Number, title = s.Title, capability = s.Capability, design = s.Design, evidence = s.Evidence,
            verification = s.Verification, references = s.References, paths = s.Paths })
    }, new JsonSerializerOptions { Encoder = System.Text.Encodings.Web.JavaScriptEncoder.UnsafeRelaxedJsonEscaping });

    private static string Text(JsonElement value, string key, int max) => CodexWorkSession.Text(value, key, max);

    internal const string PlannerInstructions =
        "\nYou are an experienced game designer and the dedicated read-only planner, not the manager or programmer. Turn requests into playable rules and testable outcomes while keeping the plan proportional to the task. Stay in this same conversation for revisions. " +
        "Use only provided read tools. Never edit, operate the editor, ask the user, approve work or delegate. " +
        "Use the manager's focused capability assessment and inspect only relevant complete EngineCapabilities.md sections and current Engine Lua declarations as needed. " +
        "Separate engine primitives, existing game logic, proposed Lua behavior and missing assets. Supported API does not mean gameplay exists or runtime tests passed. " +
        "For each requested feature describe actual observable behavior, inputs, state/physics rules, dependencies and constraints, exact affected paths and completion checks. " +
        "Include applicable input, movement/physics, camera, presentation and data as separate numbered sections only when relevant. " +
        "Cite real guide sections or source paths/lines; references must be inspected or user-supplied. Say '확인한 참고 자료 없음' when absent, never invent links or observations. " +
        "Return ONLY JSON within 22000 characters, no prose or fences. First draft schema: " +
        "{summary:string,reason:string,roles:[programmer|researcher|reviewer],sections:[{number:1,title:string,capability:string,design:string,evidence:string,verification:string,references:string,paths:[exact project relative paths]}]}. " +
        "Use Korean with normal word spacing. Summary is one or two short sentences. Each title names the work, for example 이동 구현. Start each design with one short standalone sentence describing the resulting observable behavior, preferably within 120 characters; put detailed rules, dependencies and constraints on following lines in the SAME design field. The approval list shows number. title: first design line, for example 1. 이동 구현: 방향키로 이동하고 벽에 닿으면 멈춥니다. Keep full behavior, constraints and checks in the complete fields; the short first line must accurately represent them. Maximum 12 sections numbered consecutively from 1; union of paths at most 20; combined readable field content at most 16000 characters. " +
        "Field limits: summary/reason 500, title 100, capability/evidence 600, design 1200, verification 500, references 400 characters. " +
        "On REVISION return ONLY {sections:[...]}, containing exactly the host-specified section numbers. Preserve all other sections: the host merges them unchanged. " +
        "Do not silently change another section or global scope/roles. If a requested change affects another section, put that dependency and limitation in the requested section so the manager can ask the user before expanding revision scope. " +
        "Manager reviews the result before the user sees it; no implementation begins until host plan approval.";
}
