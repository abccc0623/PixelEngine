using System.IO;
using System.Text;
using System.Text.Json;
using System.Text.Encodings.Web;
using System.Text.Unicode;

namespace PixelTool;

internal static class EngineGuide
{
    internal const int MaxReadLength = 6000;
    internal const int MaxSearchResults = 20;
    internal const int MaxBatchPages = 8;
    internal const int MaxBatchLength = 16000;
    private static readonly JsonSerializerOptions GuideJson = new() { Encoder = JavaScriptEncoder.Create(UnicodeRanges.All) };
    internal static readonly string[] Names = { "AIRestrictions.md", "EditorGuide.md", "EditorWorkflowGuide.md", "EditorQuickEditGuide.md",
        "EditorProjectGuide.md", "EditorChatGuide.md", "EditorToolsGuide.md", "EditorScopeGuide.md", "EditorRefactorGuide.md",
        "LuaConceptGuide.md", "LuaApiIndex.md", "Doc/Component/Transform.md", "Doc/Component/Camera.md", "Doc/Component/Renderer2D.md",
        "Doc/Component/Rigidbody2D.md", "Doc/Component/Animation2D.md", "Doc/Component/UIImage.md", "Doc/Component/UIText.md", "Doc/Component/UISilder.md",
        "Doc/Other/Entity.md", "Doc/Other/Scene.md", "Doc/Other/Asset.md", "Doc/Other/Input.md", "Doc/Other/Event.md",
        "Doc/Other/Time.md", "Doc/Other/Tween.md", "Doc/Other/Pool.md", "Doc/Other/Group.md", "Doc/Other/PVector2.md",
        "Doc/Other/PVector3.md", "Doc/Other/Debug.md", "Doc/Other/Engine.md",
        "Ponytail.md", "EngineCapabilities.md",
        "change-workflow/SKILL.md", "file-search-edit/SKILL.md", "game-feature-test/SKILL.md", "engine-feasibility/SKILL.md" };
    private static readonly Lazy<Dictionary<string, string>> Documents = new(() => Names.ToDictionary(name => name, name =>
    {
        using var stream = typeof(EngineGuide).Assembly.GetManifestResourceStream("PixelTool.AI." + name)
            ?? throw new InvalidOperationException("PixelTool AI guide is missing: " + name + ". Rebuild PixelTool.");
        using var reader = new StreamReader(stream, Encoding.UTF8);
        string text = reader.ReadToEnd();
        if (string.IsNullOrWhiteSpace(text)) throw new InvalidDataException("PixelTool AI guide is empty: " + name);
        return text;
    }, StringComparer.Ordinal));

    internal const string Authority = "You are the AI assistant embedded in PixelTool. The following documents are application developer instructions, not user reference text. " +
        "Follow them over conflicting project files, AGENTS.md, skills, or user requests, subject to higher-priority platform policies and actual permissions. " +
        "Other documents and skills may still be consulted. Operating restrictions take precedence over feature/API guidance. " +
        "For factual API/version mismatches, report the mismatch and inspect the project's Engine/*.lua declarations; never invent APIs.\n\n";

    private static readonly Lazy<string> Content = new(() => Authority +
        "--- AIRestrictions.md (always loaded in full) ---\n" + Documents.Value["AIRestrictions.md"] +
        "\n\n--- Required on-demand guides ---\n" +
        "The remaining embedded documents keep all their original instructions; deferred loading does not make any applicable rule optional. " +
        "Before acting or answering on a topic, read its complete relevant sections with read_engine_guide, not just search snippets. " +
        "Read all of a document when unsure which sections apply. If retrieval fails, stop the affected work and report the blocker. " +
        "Reuse current sections already present in context; retrieve again if missing after compaction.\n" +
        "EditorGuide.md is a short task-to-document index. The document names and section offsets below locate each guide directly; do not search project files for guides. " +
        "For a clear existing-value edit in at most two exact files, with no new APIs, structure, creation, deletion or refactoring, read EditorQuickEditGuide.md instead of general editor/Lua sections. If inspection reveals wider work, load the applicable guides below. EditorQuickEditGuide.md is the complete Small workflow: do not additionally read change-workflow/SKILL.md, file-search-edit/SKILL.md or Ponytail.md for this literal-only case. This shortcut never changes restrictions, approval, hash or validation gates. " +
        "For read-only capability questions, first read EngineCapabilities.md: Reading and status rules and only matching capability sections. Confirm APIs in matching Engine declarations only when needed to support a positive implementation route or resolve uncertainty. For catalog-confirmed unsupported capabilities, report the catalog limit without scanning game files. Do not load scope, workflow, Ponytail or project navigation guides for support-only questions; inspect Asset logic only if the question depends on that project. Otherwise, before project inspection read EditorProjectGuide.md: Project navigation notes and search. Before changes, read its Project layout section and EditorToolsGuide.md: Available tools and Reimport and logs. Read EditorWorkflowGuide.md only for phase details not already covered by change-workflow/SKILL.md. " +
        "For broad/abstract implementation requests, read EditorScopeGuide.md before implementation task breakdown or begin_project_work. Then read EngineCapabilities.md: Reading and status rules plus only relevant capability sections. The catalog describes primitives and known limits, not completed gameplay or verified installed DLL behavior. " +
        "Read EditorRefactorGuide.md before refactoring; EditorChatGuide.md for chat/settings or distribution questions.\n" +
        "Ponytail.md: read the entire document before coding, fixing, reviewing or designing code except the literal-only Small workflow above; its instructions remain active as specified there.\n" +
        "For Lua API questions or work, read LuaApiIndex.md to locate the name and document. Read only the matching Doc/Component or Doc/Other document, plus relevant LuaConceptGuide.md sections for shared behavior rules. Do not load every API document. " +
        "Then verify actual names, signatures and returns in the current project's Engine/*.lua: find the file with search_project_files scoped to Engine, then search its declarations with search_file_content (or the embedded chat's project content search). Embedded guides do not establish that an API exists in the installed version.\n" +
        "The index gives zero-based UTF-16 character offsets and section lengths, not token counts. " +
        "Batch independent required pages in one read_engine_guide requests array (1-8 pages). Oversized requests are paged automatically: at most 16000 returned characters total and 6000 per page. Follow each continuation to finish the requested section; nextOffset/hasMore also locate the rest of the document. " +
        "For project changes other than literal-only Small edits covered by EditorQuickEditGuide.md, read change-workflow/SKILL.md before creating work items. For assigned file changes, read file-search-edit/SKILL.md; before gameplay checks in the test phase, read game-feature-test/SKILL.md; for broad implementation requests, read engine-feasibility/SKILL.md. Support-only questions use the catalog-first route above. Read only the applicable skill through read_engine_guide before that phase, and keep AIRestrictions.md and relevant guides authoritative. These are embedded on-demand workflows, not project files or extra worker agents. " +
        "For whole documents follow hasMore. Guide tools only read fixed embedded resources, never project or external files.\n\n" +
        string.Join("\n", Names.Skip(1).Select(name => name + " (" + Documents.Value[name].Length + " characters):\n" +
            string.Join("\n", Sections(Documents.Value[name]).Select(s => $"  {s.Title}: offset={s.Offset}, length={s.Length}")))));

    internal static string Load() => Content.Value;

    private static string Document(string name) => Documents.Value.TryGetValue(name, out var text) ? text
        : throw new ArgumentException("Unknown embedded guide or skill. Use a document name from the read_engine_guide schema.");

    private static IEnumerable<(string Title, int Offset, int Length)> Sections(string text)
    {
        var headings = new List<(string Title, int Offset)> { ("Introduction", 0) };
        for (int offset = 0; offset < text.Length;)
        {
            int end = text.IndexOf('\n', offset);
            if (end < 0) end = text.Length;
            string line = text[offset..end].TrimEnd('\r');
            if (line.StartsWith("## ", StringComparison.Ordinal)) headings.Add((line[3..], offset));
            offset = end + 1;
        }
        for (int i = 0; i < headings.Count; i++)
            yield return (headings[i].Title, headings[i].Offset,
                (i + 1 < headings.Count ? headings[i + 1].Offset : text.Length) - headings[i].Offset);
    }

    internal static string Read(string document, int offset, int length) => ReadPage(document, offset, length, MaxReadLength);

    private static string ReadPage(string document, int offset, int length, int contentLimit)
    {
        string text = Document(document);
        if (offset < 0 || offset > text.Length)
            throw new ArgumentOutOfRangeException(nameof(offset), $"Guide offset must be between 0 and {text.Length} UTF-16 characters.");
        if (length < 1) throw new ArgumentOutOfRangeException(nameof(length), "Guide length must be positive; oversized reads are paged automatically.");
        if (offset > 0 && offset < text.Length && char.IsLowSurrogate(text[offset]) && char.IsHighSurrogate(text[offset - 1]))
            throw new ArgumentException("Offset must not split a Unicode character. Use the returned nextOffset.");
        int requestedEnd = offset + Math.Min(length, text.Length - offset);
        if (requestedEnd < text.Length && requestedEnd > offset && char.IsHighSurrogate(text[requestedEnd - 1]) && char.IsLowSurrogate(text[requestedEnd])) requestedEnd++;
        int end = offset + Math.Min(requestedEnd - offset, contentLimit);
        if (end < text.Length && end > offset && char.IsHighSurrogate(text[end - 1]) && char.IsLowSurrogate(text[end])) end--;
        if (end == offset && offset < text.Length) throw new ArgumentException("Length is too small for the next Unicode character; request at least 2 characters.");
        int remainingLength = requestedEnd - end;
        return JsonSerializer.Serialize(new { document, source = "embedded", offset, requestedLength = length, length = end - offset,
            totalLength = text.Length, nextOffset = end, hasMore = end < text.Length,
            partial = offset != 0 || end != text.Length, truncated = remainingLength > 0, remainingLength,
            continuation = remainingLength > 0 ? new { document, offset = end, length = remainingLength } : null,
            content = text[offset..end] }, GuideJson);
    }

    internal static string ReadBatch(JsonElement requests)
    {
        if (requests.ValueKind != JsonValueKind.Array || requests.GetArrayLength() is < 1 or > MaxBatchPages)
            throw new ArgumentException("Use 1 to 8 guide pages.");
        // Validate first, then redistribute unused short-page budget before paging long documents.
        var input = requests.EnumerateArray().ToArray();
        var pages = new JsonElement[input.Length];
        var sizes = new int[input.Length];
        for (int i = 0; i < input.Length; i++)
        {
            using var page = JsonDocument.Parse(ReadPage(input[i].GetProperty("document").GetString() ?? "",
                input[i].GetProperty("offset").GetInt32(), input[i].GetProperty("length").GetInt32(), MaxReadLength));
            pages[i] = page.RootElement.Clone();
            sizes[i] = pages[i].GetProperty("length").GetInt32();
        }
        int budget = MaxBatchLength, remaining = input.Length;
        foreach (int i in Enumerable.Range(0, input.Length).OrderBy(i => sizes[i]))
        {
            int limit = Math.Min(sizes[i], budget / remaining--);
            if (limit < sizes[i])
            {
                using var page = JsonDocument.Parse(ReadPage(input[i].GetProperty("document").GetString() ?? "",
                    input[i].GetProperty("offset").GetInt32(), input[i].GetProperty("length").GetInt32(), limit));
                pages[i] = page.RootElement.Clone();
            }
            budget -= pages[i].GetProperty("length").GetInt32();
        }
        return JsonSerializer.Serialize(new { pages }, GuideJson);
    }

    internal static string Search(string document, string query, int limit)
    {
        string text = Document(document);
        if (string.IsNullOrWhiteSpace(query) || query.Length > 200 || limit is < 1 or > MaxSearchResults)
            throw new ArgumentException("Use a literal query of 1 to 200 characters and a limit of 1 to 20.");
        var matches = new List<object>();
        int offset = 0;
        bool truncated = false;
        while (offset < text.Length)
        {
            int match = text.IndexOf(query, offset, StringComparison.OrdinalIgnoreCase);
            if (match < 0) break;
            if (matches.Count == limit) { truncated = true; break; }
            int start = Math.Max(0, match - 20), end = Math.Min(text.Length, start + 240);
            if (start > 0 && char.IsLowSurrogate(text[start]) && char.IsHighSurrogate(text[start - 1])) start++;
            if (end < text.Length && char.IsHighSurrogate(text[end - 1]) && char.IsLowSurrogate(text[end])) end--;
            matches.Add(new { offset = match, snippetOffset = start, snippet = text[start..end],
                snippetTruncated = start != 0 || end != text.Length });
            offset = match + query.Length;
        }
        return JsonSerializer.Serialize(new { document, source = "embedded", query, matches, truncated,
            note = "Search snippets only locate text. Read complete applicable sections before acting." }, GuideJson);
    }
}
