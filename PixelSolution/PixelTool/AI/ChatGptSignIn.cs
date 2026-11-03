using System.IO;
using System.Net;
using System.Net.Http;
using System.Net.Sockets;
using System.Security.AccessControl;
using System.Security.Cryptography;
using System.Security.Principal;
using System.Text;
using System.Text.Json;

namespace PixelTool;

// Public-client OAuth: never read Codex credentials or accept an API key.
internal sealed class ChatGptSignIn : IDisposable
{
    private const string Issuer = "https://auth.openai.com";
    private const string Resource = "https://api.openai.com/v1";
    private const string RequiredScope = "chatgpt.tokens.use.direct";
    private readonly HttpClient _http = new(new HttpClientHandler { AllowAutoRedirect = false }) { Timeout = TimeSpan.FromSeconds(40) };
    private readonly CancellationTokenSource _stop = new();
    private readonly SemaphoreSlim _gate = new(1);
    private readonly string _directory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "PixelTool", "ChatGPT");
    private HttpListener? _listener;
    private Task? _login;
    internal event Action<bool>? LoginCompleted;
    internal string? LoginError { get; private set; }
    internal Session? Current { get; private set; }
    internal static bool HasSavedAccount => File.Exists(Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "PixelTool", "ChatGPT", "active.txt"));
    internal string? CurrentKey => Current == null ? null : AccountKey(Current);

    internal object[] Accounts() => Directory.EnumerateFiles(_directory, "*.bin").Select(path =>
    {
        var session = ReadSession(Path.GetFileNameWithoutExtension(path));
        return (object)new { id = AccountKey(session), label = session.Email + " · " + session.ClientId[^Math.Min(8, session.ClientId.Length)..] };
    }).ToArray();

    internal void SelectAccount(string key)
    {
        if (_login is { IsCompleted: false }) throw new InvalidOperationException("로그인 완료 후 계정을 변경하세요.");
        var selected = ReadSession(key);
        File.WriteAllText(Path.Combine(_directory, "active.txt"), key);
        Current = selected;
    }

    private Session ReadSession(string key)
    {
        if (!Guid.TryParseExact(key, "N", out _)) throw new InvalidDataException("잘못된 계정 식별자입니다.");
        var session = JsonSerializer.Deserialize<Session>(ProtectedData.Unprotect(File.ReadAllBytes(Path.Combine(_directory, key + ".bin")), null, DataProtectionScope.CurrentUser))
            ?? throw new InvalidDataException("저장된 계정 정보가 잘못되었습니다.");
        if (AccountKey(session) != key) throw new InvalidDataException("계정 등록 정보가 일치하지 않습니다.");
        return session;
    }

    internal sealed class Session
    {
        public string ClientId { get; set; } = "";
        public string Subject { get; set; } = "";
        public string Email { get; set; } = "";
        public string AccessToken { get; set; } = "";
        public string RefreshToken { get; set; } = "";
        public string IdToken { get; set; } = "";
        public string Scope { get; set; } = "";
        public DateTimeOffset ExpiresAt { get; set; }
    }

    internal ChatGptSignIn()
    {
        var folder = Directory.CreateDirectory(_directory);
        var security = new DirectorySecurity();
        var user = WindowsIdentity.GetCurrent().User ?? throw new InvalidOperationException("Windows account is unavailable.");
        security.SetAccessRuleProtection(true, false);
        security.SetOwner(user);
        security.AddAccessRule(new FileSystemAccessRule(user, FileSystemRights.FullControl,
            InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit, PropagationFlags.None, AccessControlType.Allow));
        folder.SetAccessControl(security);
        string activePath = Path.Combine(_directory, "active.txt");
        if (File.Exists(activePath))
        {
            string key = File.ReadAllText(activePath);
            if (!Guid.TryParseExact(key, "N", out _)) throw new InvalidDataException("ChatGPT account selection is invalid.");
            Current = ReadSession(key);
        }
    }

    internal async Task<string> BeginLoginAsync(bool newAccount)
    {
        if (_login is { IsCompleted: false }) throw new InvalidOperationException("브라우저 로그인이 이미 진행 중입니다.");
        string hostPath = Path.Combine(_directory, "host.txt");
        string host;
        using (var file = new FileStream(hostPath, FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None))
        {
            using var reader = new StreamReader(file, Encoding.UTF8, leaveOpen: true);
            host = reader.ReadToEnd();
            if (host.Length == 0)
            {
                host = "urn:uuid:" + Guid.NewGuid();
                file.Write(Encoding.UTF8.GetBytes(host));
                file.Flush(true);
            }
            if (!host.StartsWith("urn:uuid:") || !Guid.TryParse(host[9..], out _)) throw new InvalidDataException("ChatGPT host ID is invalid.");
        }
        Session? previous = newAccount ? null : Current;
        string state = RandomValue(), nonce = RandomValue(), verifier = RandomValue();
        // Reserve an ephemeral loopback port; retry if another process takes it before HttpListener.
        HttpListener? listener = null;
        string callback = "";
        for (int attempt = 0; attempt < 5; attempt++)
        {
            var probe = new TcpListener(IPAddress.Loopback, 0);
            probe.Start();
            int port = ((IPEndPoint)probe.LocalEndpoint).Port;
            probe.Stop();
            listener = new HttpListener();
            listener.Prefixes.Add($"http://127.0.0.1:{port}/");
            try { listener.Start(); callback = $"http://127.0.0.1:{port}/auth/callback"; break; }
            catch (HttpListenerException) { listener.Close(); listener = null; }
        }
        if (listener == null) throw new IOException("ChatGPT 로그인용 로컬 연결을 열지 못했습니다.");
        _listener = listener;
        var query = new Dictionary<string, string>
        {
            ["client_id"] = previous?.ClientId ?? "dynamic_agent_client",
            ["ext_agent_host_id"] = host, ["response_type"] = "code", ["redirect_uri"] = callback,
            ["scope"] = "openid profile email offline_access resource.invoke " + RequiredScope,
            ["resource"] = Resource, ["state"] = state, ["nonce"] = nonce,
            ["code_challenge_method"] = "S256", ["code_challenge"] = Base64Url(SHA256.HashData(Encoding.ASCII.GetBytes(verifier)))
        };
        if (previous == null) query["agent_name_hint"] = "PixelTool";
        // Do not put even an ID-token hint in a browser-launch URL/command line.
        else if (previous.Email.Length > 0) query["login_hint"] = previous.Email;
        _login = CompleteLoginAsync(listener, callback, state, nonce, verifier, previous);
        await Task.CompletedTask;
        return Issuer + "/api/accounts/authorize?" + string.Join("&", query.Select(p => Uri.EscapeDataString(p.Key) + "=" + Uri.EscapeDataString(p.Value)));
    }

    private async Task CompleteLoginAsync(HttpListener listener, string callback, string state, string nonce, string verifier, Session? previous)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(_stop.Token);
        timeout.CancelAfter(TimeSpan.FromMinutes(5));
        using var close = timeout.Token.Register(listener.Close);
        bool success = false;
        try
        {
            HttpListenerContext context;
            while (true)
            {
                context = await listener.GetContextAsync().ConfigureAwait(false);
                if (context.Request.HttpMethod == "GET" && context.Request.Url?.AbsolutePath == "/auth/callback" &&
                    !context.Request.Headers.AllKeys.Contains("Origin") &&
                    context.Request.QueryString.GetValues("state") is { Length: 1 } states && states[0] == state) break;
                context.Response.StatusCode = 400;
                context.Response.Close();
            }
            var query = context.Request.QueryString;
            // The browser gets no credentials, OAuth code or detailed diagnostics back.
            byte[] page = Encoding.UTF8.GetBytes("<html><meta charset='utf-8'><p>PixelTool로 돌아가 로그인 결과를 확인하세요. 이 창은 닫아도 됩니다.</p></html>");
            context.Response.ContentType = "text/html; charset=utf-8";
            context.Response.Headers["Cache-Control"] = "no-store";
            await context.Response.OutputStream.WriteAsync(page, timeout.Token).ConfigureAwait(false);
            context.Response.Close();
            if (query["error"] != null) throw new InvalidOperationException("ChatGPT 로그인 또는 사용 권한 승인이 취소되었습니다.");
            string clientId = query["client_id"] ?? previous?.ClientId ?? "";
            if (clientId.Length == 0 || clientId == "dynamic_agent_client" || (previous != null && previous.ClientId != clientId))
                throw new InvalidDataException("ChatGPT 앱 등록 결과가 일치하지 않습니다.");
            var tokens = await TokenAsync(new()
            {
                ["grant_type"] = "authorization_code", ["client_id"] = clientId,
                ["code"] = query["code"] ?? throw new InvalidDataException("인증 코드가 없습니다."),
                ["code_verifier"] = verifier, ["redirect_uri"] = callback, ["resource"] = Resource
            }, timeout.Token).ConfigureAwait(false);
            var identity = await ValidateIdTokenAsync(tokens.GetProperty("id_token").GetString()!, clientId, nonce, timeout.Token).ConfigureAwait(false);
            if (previous != null && identity.Subject != previous.Subject) throw new InvalidDataException("선택한 ChatGPT 계정과 로그인 결과가 다릅니다. 다른 계정 추가를 사용하세요.");
            var session = new Session { ClientId = clientId, Subject = identity.Subject, Email = identity.Email };
            ApplyTokens(session, tokens);
            await SaveAsync(session, timeout.Token).ConfigureAwait(false);
            Current = session;
            LoginError = null;
            success = true;
        }
        catch (Exception ex)
        {
            // Do not expose HTTP bodies, callback URLs, JWTs or authorization codes.
            LoginError = ex is OperationCanceledException or HttpListenerException or ObjectDisposedException
                ? "로그인이 취소되었거나 제한 시간이 지났습니다."
                : ex is InvalidDataException or InvalidOperationException ? ex.Message : "ChatGPT 로그인 통신 또는 인증 검증에 실패했습니다.";
        }
        finally
        {
            listener.Close();
            if (!_stop.IsCancellationRequested) LoginCompleted?.Invoke(success);
        }
    }

    internal async Task<string> AccessTokenAsync(CancellationToken token)
    {
        await _gate.WaitAsync(token).ConfigureAwait(false);
        try
        {
            var session = Current ?? throw new InvalidOperationException("ChatGPT 로그인이 필요합니다.");
            if (session.ExpiresAt > DateTimeOffset.UtcNow.AddMinutes(2)) return session.AccessToken;
            if (session.RefreshToken.Length == 0) throw new InvalidOperationException("로그인을 갱신해야 합니다. 계정 연결을 다시 누르세요.");
            // A separate file lock also prevents different editor processes racing a rotating refresh token.
            using var refreshLock = await LockAsync(token).ConfigureAwait(false);
            string key = AccountKey(session);
            string path = Path.Combine(_directory, key + ".bin");
            var latest = JsonSerializer.Deserialize<Session>(ProtectedData.Unprotect(File.ReadAllBytes(path), null, DataProtectionScope.CurrentUser))
                ?? throw new InvalidDataException("저장된 ChatGPT 계정이 잘못되었습니다.");
            if (latest.ClientId != session.ClientId || latest.Subject != session.Subject) throw new InvalidDataException("계정 정보가 일치하지 않습니다.");
            if (latest.ExpiresAt <= DateTimeOffset.UtcNow.AddMinutes(2))
            {
                var replacement = await TokenAsync(new() { ["grant_type"] = "refresh_token", ["client_id"] = latest.ClientId,
                    ["refresh_token"] = latest.RefreshToken, ["resource"] = Resource }, token).ConfigureAwait(false);
                ApplyTokens(latest, replacement);
                await SaveAsync(latest, token).ConfigureAwait(false);
            }
            Current = latest;
            return latest.AccessToken;
        }
        finally { _gate.Release(); }
    }

    private async Task<FileStream> LockAsync(CancellationToken token)
    {
        for (int i = 0; i < 100; i++)
        {
            try { return new FileStream(Path.Combine(_directory, "refresh.lock"), FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None); }
            catch (IOException) { await Task.Delay(100, token).ConfigureAwait(false); }
        }
        throw new IOException("다른 에디터의 계정 갱신이 끝나지 않았습니다. 잠시 후 다시 연결하세요.");
    }

    private async Task<JsonElement> TokenAsync(Dictionary<string, string> fields, CancellationToken token)
    {
        using var response = await _http.PostAsync(Issuer + "/api/accounts/oauth/token", new FormUrlEncodedContent(fields), token).ConfigureAwait(false);
        if (!response.IsSuccessStatusCode) throw new InvalidOperationException($"ChatGPT 인증 갱신에 실패했습니다 (HTTP {(int)response.StatusCode}). 계정 연결을 다시 시도하세요.");
        using var document = JsonDocument.Parse(await response.Content.ReadAsStringAsync(token).ConfigureAwait(false));
        return document.RootElement.Clone();
    }

    private static void ApplyTokens(Session session, JsonElement tokens)
    {
        string scope = tokens.TryGetProperty("scope", out var granted) ? granted.GetString() ?? "" : session.Scope;
        if (!scope.Split(' ').Contains(RequiredScope)) throw new InvalidOperationException("ChatGPT 요금제 사용 권한이 승인되지 않았습니다. 브라우저에서 권한을 확인하세요.");
        if (tokens.GetProperty("token_type").GetString()?.Equals("Bearer", StringComparison.OrdinalIgnoreCase) != true)
            throw new InvalidDataException("지원하지 않는 ChatGPT 인증 형식입니다.");
        session.AccessToken = tokens.GetProperty("access_token").GetString() ?? throw new InvalidDataException("Access token missing.");
        if (tokens.TryGetProperty("refresh_token", out var refresh)) session.RefreshToken = refresh.GetString() ?? "";
        // A retained ID token is only a login hint. Identity was validated on authorization above.
        if (tokens.TryGetProperty("id_token", out var id)) session.IdToken = id.GetString() ?? "";
        session.Scope = scope;
        session.ExpiresAt = DateTimeOffset.UtcNow.AddSeconds(tokens.GetProperty("expires_in").GetInt32());
    }

    private async Task<(string Subject, string Email)> ValidateIdTokenAsync(string jwt, string clientId, string nonce, CancellationToken token)
    {
        string[] parts = jwt.Split('.');
        if (parts.Length != 3) throw new InvalidDataException("잘못된 로그인 토큰입니다.");
        using var header = JsonDocument.Parse(Decode(parts[0]));
        using var payload = JsonDocument.Parse(Decode(parts[1]));
        if (header.RootElement.GetProperty("alg").GetString() != "RS256") throw new InvalidDataException("지원하지 않는 로그인 서명 방식입니다.");
        using var discovery = JsonDocument.Parse(await _http.GetStringAsync(Issuer + "/.well-known/openid-configuration", token).ConfigureAwait(false));
        if (discovery.RootElement.GetProperty("issuer").GetString() != Issuer) throw new InvalidDataException("로그인 발급자가 다릅니다.");
        var jwksUri = new Uri(discovery.RootElement.GetProperty("jwks_uri").GetString()!);
        if (jwksUri.Scheme != "https" || jwksUri.Host != "auth.openai.com") throw new InvalidDataException("신뢰할 수 없는 로그인 서명 키 주소입니다.");
        using var keys = JsonDocument.Parse(await _http.GetStringAsync(jwksUri, token).ConfigureAwait(false));
        var key = keys.RootElement.GetProperty("keys").EnumerateArray().First(k =>
            k.GetProperty("kid").GetString() == header.RootElement.GetProperty("kid").GetString() && k.GetProperty("kty").GetString() == "RSA");
        using var rsa = RSA.Create();
        rsa.ImportParameters(new RSAParameters { Modulus = Decode(key.GetProperty("n").GetString()!), Exponent = Decode(key.GetProperty("e").GetString()!) });
        if (!rsa.VerifyData(Encoding.ASCII.GetBytes(parts[0] + "." + parts[1]), Decode(parts[2]), HashAlgorithmName.SHA256, RSASignaturePadding.Pkcs1))
            throw new InvalidDataException("로그인 서명이 올바르지 않습니다.");
        var claims = payload.RootElement;
        var audience = claims.GetProperty("aud");
        bool validAudience = audience.ValueKind == JsonValueKind.String ? audience.GetString() == clientId
            : audience.ValueKind == JsonValueKind.Array && audience.EnumerateArray().Any(a => a.GetString() == clientId);
        long now = DateTimeOffset.UtcNow.ToUnixTimeSeconds();
        if (!validAudience || claims.GetProperty("iss").GetString() != Issuer || claims.GetProperty("nonce").GetString() != nonce ||
            claims.GetProperty("exp").GetInt64() <= now || (claims.TryGetProperty("nbf", out var nbf) && nbf.GetInt64() > now + 30) ||
            (claims.TryGetProperty("azp", out var azp) && azp.GetString() != clientId) ||
            (audience.ValueKind == JsonValueKind.Array && audience.GetArrayLength() > 1 && !claims.TryGetProperty("azp", out _)))
            throw new InvalidDataException("로그인 토큰의 계정·유효기간·요청 정보가 일치하지 않습니다.");
        string subject = claims.GetProperty("sub").GetString() ?? "";
        if (subject.Length == 0) throw new InvalidDataException("로그인 계정 식별자가 없습니다.");
        return (subject, claims.TryGetProperty("email", out var email) ? email.GetString() ?? "" : "");
    }

    private async Task SaveAsync(Session session, CancellationToken token)
    {
        string key = AccountKey(session);
        byte[] encrypted = ProtectedData.Protect(JsonSerializer.SerializeToUtf8Bytes(session), null, DataProtectionScope.CurrentUser);
        string path = Path.Combine(_directory, key + ".bin");
        string temporary = path + "." + Guid.NewGuid().ToString("N") + ".tmp";
        await File.WriteAllBytesAsync(temporary, encrypted, token).ConfigureAwait(false);
        File.Move(temporary, path, true);
        await File.WriteAllTextAsync(Path.Combine(_directory, "active.txt"), key, token).ConfigureAwait(false);
    }

    private static string AccountKey(Session session) => new Guid(SHA256.HashData(Encoding.UTF8.GetBytes(session.ClientId + "\n" + session.Subject))[..16]).ToString("N");
    private static string RandomValue() => Base64Url(RandomNumberGenerator.GetBytes(32));
    private static string Base64Url(byte[] value) => Convert.ToBase64String(value).TrimEnd('=').Replace('+', '-').Replace('/', '_');
    private static byte[] Decode(string value) => Convert.FromBase64String(value.Replace('-', '+').Replace('_', '/') + new string('=', (4 - value.Length % 4) % 4));

    public void Dispose()
    {
        _stop.Cancel();
        _listener?.Close();
        _http.Dispose();
    }
}
