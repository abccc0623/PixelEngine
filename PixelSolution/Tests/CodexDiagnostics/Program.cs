using System.IO;
using System.Reflection;
using System.Text.Json;

if (args.FirstOrDefault() == "app-server")
{
    string mode = Environment.GetEnvironmentVariable("PIXELTOOL_DIAGNOSTIC_FIXTURE") ?? "";
    string? request = await Console.In.ReadLineAsync();
    if (mode == "startup")
    {
        await Console.Error.WriteLineAsync("startup diagnostic failure");
        return 23;
    }
    using var json = JsonDocument.Parse(request!);
    await Console.Out.WriteLineAsync(JsonSerializer.Serialize(new { id = json.RootElement.GetProperty("id").GetInt64(), result = new { } }));
    await Console.Out.FlushAsync();
    await Console.In.ReadLineAsync(); // initialized notification
    if (mode == "wait") { await Task.Delay(TimeSpan.FromSeconds(20)); return 0; }
    if (mode == "invalid")
    {
        await Console.Out.WriteLineAsync("not-json-stdout-private-marker");
        await Console.Out.FlushAsync();
        await Task.Delay(TimeSpan.FromSeconds(20));
        return 0;
    }
    for (int i = 0; i < 25; i++) await Console.Error.WriteLineAsync("old warning " + i);
    await Console.Error.WriteLineAsync("Authorization: Bearer test-bearer-secret");
    await Console.Error.WriteLineAsync("OPENAI_API_KEY=sk-fake-sensitive-key");
    await Console.Error.WriteLineAsync("{\"refresh_token\":\"test-refresh-secret\"}");
    await Console.Error.WriteLineAsync("cookie: session=test-cookie-secret");
    await Console.Error.WriteLineAsync("eyJhbGciOiJIUzI1NiJ9.dGVzdA.signature");
    await Console.Error.WriteLineAsync("https://example.invalid/callback?code=test-url-secret");
    await Console.Error.WriteLineAsync(new string('a', 64));
    await Console.Error.WriteLineAsync("actual fixture error: missing settings");
    return 23;
}

var assembly = Assembly.LoadFrom(Path.GetFullPath(args[0]));
var type = assembly.GetType("PixelTool.CodexChatClient", true)!;
string executable = Path.Combine(AppContext.BaseDirectory, "CodexDiagnostics.exe");
void Check(bool value, string message) { if (!value) throw new Exception(message); Console.WriteLine("PASS " + message); }
Task Connect(object client) => (Task)type.GetMethod("ConnectAsync", new[] { typeof(string), typeof(string), typeof(bool) })!
    .Invoke(client, new object[] { executable, Environment.CurrentDirectory, false })!;
string? previous = Environment.GetEnvironmentVariable("PIXELTOOL_DIAGNOSTIC_FIXTURE");
try
{
    var sanitizer = type.GetMethod("SanitizeDiagnostic", BindingFlags.NonPublic | BindingFlags.Static)!;
    string cleaned = (string)sanitizer.Invoke(null, new object[] { "access_token=synthetic-access-secret id_token=synthetic-id-secret client_secret=synthetic-client-secret password=synthetic-password\nuseful explanation" })!;
    Check(!cleaned.Contains("synthetic-") && cleaned.Contains("useful explanation"), "redacts credential assignments but keeps useful errors");
    foreach (string mode in new[] { "exit", "startup", "invalid", "wait" })
    {
        Environment.SetEnvironmentVariable("PIXELTOOL_DIAGNOSTIC_FIXTURE", mode);
        var client = (IAsyncDisposable)Activator.CreateInstance(type, true)!;
        var disconnected = new TaskCompletionSource<string>(TaskCreationOptions.RunContinuationsAsynchronously);
        Action<string> handler = details => disconnected.TrySetResult(details);
        type.GetEvent("Disconnected")!.AddEventHandler(client, handler);
        try
        {
            string? connectionError = null;
            try { await Connect(client).WaitAsync(TimeSpan.FromSeconds(10)); }
            catch (IOException ex) { connectionError = ex.Message; }
            if (mode == "wait")
            {
                await client.DisposeAsync();
                Check(!disconnected.Task.IsCompleted, "intentional disposal does not report a connection failure");
                continue;
            }
            string report = connectionError ?? await disconnected.Task.WaitAsync(TimeSpan.FromSeconds(8));
            if (mode == "invalid")
                Check(report.Contains("JSON 응답 해석") && report.Contains("확인 불가"), "protocol errors report their cause without inventing an exit code: " + report);
            else
                Check(report.Contains("23 (0x00000017)") && report.Contains(mode == "startup" ? "startup diagnostic failure" : "actual fixture error"), mode + " preserves exit code and trailing stderr");
            Check(report.Length < 6500 && !report.Contains("old warning 0\n"), "diagnostic output is bounded");
            Check(!report.Contains("stdout-private-marker"), "stdout payloads are not retained in diagnostics");
            foreach (string secret in new[] { "test-bearer-secret", "sk-fake-sensitive-key", "test-refresh-secret", "test-cookie-secret", "eyJhbGciOiJIUzI1NiJ9", "test-url-secret", new string('a', 64) })
                Check(!report.Contains(secret), "credential is not exposed");
        }
        finally { await client.DisposeAsync(); }
    }
}
finally { Environment.SetEnvironmentVariable("PIXELTOOL_DIAGNOSTIC_FIXTURE", previous); }
Console.WriteLine("PASS: child-process diagnostic tests; no real Codex, account or inference used.");
return 0;
