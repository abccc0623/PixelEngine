using System.Reflection;
using System.Text.Json;
using System.IO;

// Run after building PixelTool. Uses one small inference only with --turn.
string assemblyPath = Path.GetFullPath(args[0]);
var assembly = Assembly.LoadFrom(assemblyPath);
var type = assembly.GetType("PixelTool.CodexEditorClient", throwOnError: true)!;
var client = (IAsyncDisposable)Activator.CreateInstance(type, nonPublic: true)!;
var folder = Directory.CreateTempSubdirectory("PixelTool-CliSmoke-").FullName;
Directory.CreateDirectory(Path.Combine(folder, "Asset"));
File.WriteAllText(Path.Combine(folder, "Asset", "main.lua"), "-- CLI_SMOKE_OK\n");
string? threadId = null;
async Task<JsonElement> Request(string method, object parameters)
{
    var task = (Task<JsonElement>)type.GetMethod("RequestAsync")!.Invoke(client, new[] { method, parameters })!;
    return await task.WaitAsync(TimeSpan.FromSeconds(100));
}
try
{
    await ((Task)type.GetMethod("ConnectAsync")!.Invoke(client, new object[] { folder })!).WaitAsync(TimeSpan.FromSeconds(30));
    var account = await Request("account/read", new { refreshToken = false });
    if (account.GetProperty("account").GetProperty("type").GetString() != "chatgpt") throw new Exception("ChatGPT login required.");
    var accounts = await Request("account/list", new { });
    if (accounts.GetProperty("data").GetArrayLength() == 0) throw new Exception("No CLI account sessions.");
    var models = await Request("model/list", new { includeHidden = false, limit = 100 });
    if (models.GetProperty("data").GetArrayLength() == 0) throw new Exception("No models.");
    Console.WriteLine("PASS: app-server initialize, ChatGPT account, account sessions, model catalog");
    var started = await Request("thread/start", new { cwd = folder, developerInstructions = "Read only. Do not change files.", sandbox = "read-only" });
    threadId = started.GetProperty("thread").GetProperty("id").GetString()!;
    Console.WriteLine("PASS: thread/start with editor dynamic tool definitions");
    if (args.Contains("--turn"))
    {
        var completed = new TaskCompletionSource<JsonElement>(TaskCreationOptions.RunContinuationsAsynchronously);
        bool usedTool = false;
        string answer = "";
        Action<string, JsonElement> handler = (method, p) =>
        {
            if (method == "item/completed" && p.TryGetProperty("item", out var item))
            {
                if (item.GetProperty("type").GetString() == "dynamicToolCall") usedTool = item.GetProperty("success").GetBoolean();
                if (item.GetProperty("type").GetString() == "agentMessage") answer += item.GetProperty("text").GetString();
            }
            if (method == "turn/completed") completed.TrySetResult(p.GetProperty("turn").Clone());
        };
        type.GetEvent("Notification")!.AddEventHandler(client, handler);
        var catalog = models.GetProperty("data").EnumerateArray().ToArray();
        string model = catalog.FirstOrDefault(m => m.GetProperty("model").GetString() == "gpt-6.1-sol") is var preferred && preferred.ValueKind != JsonValueKind.Undefined
            ? preferred.GetProperty("model").GetString()! : catalog[0].GetProperty("model").GetString()!;
        await Request("turn/start", new { threadId, model, confirmActions = false,
            sandboxPolicy = new { type = "readOnly" }, input = new[] { new { type = "text",
                text = "Call pixeltool.read_project_file_lines for Asset/main.lua (startLine 1, lineCount 5), then reply only the marker in that file. Do not call other tools." } } });
        var turn = await completed.Task.WaitAsync(TimeSpan.FromMinutes(2));
        if (turn.GetProperty("status").GetString() != "completed" || !usedTool || !answer.Contains("CLI_SMOKE_OK"))
            throw new Exception("Dynamic tool inference did not complete correctly: " + turn.GetRawText());
        Console.WriteLine("PASS: real streamed turn, editor file-read tool roundtrip, final response");
        await Request("thread/resume", new { threadId, cwd = folder, developerInstructions = "Read only. Do not change files.", sandbox = "read-only" });
        Console.WriteLine("PASS: thread/resume after persisted turn");
    }
}
finally
{
    if (threadId != null) { try { await Request("thread/archive", new { threadId }); } catch { } }
    await client.DisposeAsync();
    // Delete only the exact file and directories this test created.
    File.Delete(Path.Combine(folder, "Asset", "main.lua"));
    Directory.Delete(Path.Combine(folder, "Asset"));
    try { Directory.Delete(folder); } catch (IOException) { }
}
