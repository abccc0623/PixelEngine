using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

namespace PixelTool;

internal sealed class CodexChatHistory
{
    public string ProjectPath { get; set; } = "";
    public string? ThreadId { get; set; }
    public int ToolSchemaVersion { get; set; }
    public List<string> PreviousThreadIds { get; set; } = new();
    internal const int CurrentToolSchemaVersion = 23;
    public string Executable { get; set; } = "codex.exe";
    public string? Model { get; set; }
    public string? ProgrammerModel { get; set; }
    public string? ProgrammerEffort { get; set; }
    public string? PlannerModel { get; set; }
    public string? PlannerEffort { get; set; }
    public string? ArtistModel { get; set; }
    public string? ArtistEffort { get; set; }
    public List<CodexChatMessage> Messages { get; set; } = new();

    internal static string PathFor(string project)
    {
        string identity = Path.GetFullPath(project).TrimEnd(Path.DirectorySeparatorChar).ToUpperInvariant();
        string key = Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(identity)));
        return Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "PixelTool", "AIChat", key + ".json");
    }

    internal static CodexChatHistory Load(string project)
    {
        string path = PathFor(project);
        if (!File.Exists(path)) return new() { ProjectPath = project };
        var result = JsonSerializer.Deserialize<CodexChatHistory>(File.ReadAllText(path)) ?? throw new InvalidDataException("Chat history is invalid.");
        if (!string.Equals(Path.GetFullPath(result.ProjectPath), Path.GetFullPath(project), StringComparison.OrdinalIgnoreCase))
            throw new InvalidDataException("Chat history belongs to another project.");
        return result;
    }

    internal void Save()
    {
        string path = PathFor(ProjectPath);
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        string temporary = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
        File.WriteAllText(temporary, JsonSerializer.Serialize(this), new UTF8Encoding(false));
        File.Move(temporary, path, overwrite: true);
    }

    internal CodexChatHistory Snapshot() => new()
    {
        ProjectPath = ProjectPath, ThreadId = ThreadId, ToolSchemaVersion = ToolSchemaVersion,
        PreviousThreadIds = new List<string>(PreviousThreadIds), Executable = Executable,
        Model = Model, ProgrammerModel = ProgrammerModel, ProgrammerEffort = ProgrammerEffort,
        PlannerModel = PlannerModel, PlannerEffort = PlannerEffort, ArtistModel = ArtistModel, ArtistEffort = ArtistEffort,
        Messages = Messages.Select(m => new CodexChatMessage
        {
            Role = m.Role, Text = m.Text, ItemId = m.ItemId, Details = m.Details
        }).ToList()
    };
}

internal sealed class CodexChatMessage
{
    public string Role { get; set; } = "";
    public string Text { get; set; } = "";
    public string? ItemId { get; set; }
    public string? Details { get; set; }
}
