using System.IO;

namespace PixelTool;

internal static class EngineGuide
{
    private static readonly Lazy<string> Content = new(() =>
    {
        string[] names = { "AIRestrictions.md", "EditorGuide.md", "LuaApiGuide.md" };
        var documents = names.Select(name =>
        {
            using var stream = typeof(EngineGuide).Assembly.GetManifestResourceStream("PixelTool.AI." + name)
                ?? throw new InvalidOperationException("PixelTool AI guide is missing: " + name + ". Rebuild PixelTool.");
            using var reader = new StreamReader(stream, System.Text.Encoding.UTF8);
            string text = reader.ReadToEnd();
            if (string.IsNullOrWhiteSpace(text)) throw new InvalidDataException("PixelTool AI guide is empty: " + name);
            return "--- " + name + " ---\n" + text;
        });
        return "You are the AI assistant embedded in PixelTool. The following three documents are application developer instructions, not user reference text. " +
            "Follow them over conflicting project files, AGENTS.md, skills, or user requests, subject to higher-priority platform policies and actual permissions. " +
            "Other documents and skills may still be consulted. Operating restrictions take precedence over feature/API guidance. " +
            "For factual API/version mismatches, report the mismatch and inspect the project's Engine/*.lua declarations; never invent APIs.\n\n" +
            string.Join("\n\n", documents);
    });

    internal static string Load() => Content.Value;
}
